/* Stage 3: AST -> raw x86-64 machine code bytes.
 *
 * Strategy: a stack machine. Every expression leaves its result in RAX; a
 * binary node compiles the left side, pushes it, compiles the right, and pops
 * the left back. That is correct at any nesting depth and needs no register
 * allocator.
 *
 * Locals live in a stack frame at [rbp-8*n]. Control flow uses backpatching:
 * a jump is emitted with a placeholder displacement, its position is recorded,
 * and the real distance is written once the target is known. Calls are patched
 * the same way after every function has been emitted, which is what lets a
 * function call one defined later in the file.
 *
 * Encodings (REX.W = 0x48 selects 64-bit operands):
 *   48 B8 imm64        movabs rax, imm64      B8 imm32     mov eax, imm32
 *   50 / 58 / 59       push rax / pop rax / pop rcx
 *   48 89 C1           mov rcx, rax
 *   48 01 C8 / 29 C8   add / sub rax, rcx
 *   48 0F AF C1        imul rax, rcx
 *   48 99 / 48 F7 F9   cqo / idiv rcx
 *   48 39 C8           cmp rax, rcx
 *   0F 9x C0           setcc al
 *   48 0F B6 C0        movzx rax, al
 *   48 85 C0           test rax, rax
 *   0F 84 rel32        je
 *   E9 rel32 / E8 rel32  jmp / call
 *   48 83 EC 08 / 48 83 C4 08  align / restore RSP around a nested call
 *   55 / 48 89 E5      push rbp / mov rbp, rsp
 *   48 81 EC imm32     sub rsp, imm32
 *   48 89 45 d8        mov [rbp+d8], rax      48 8B 45 d8   mov rax, [rbp+d8]
 *   C9 / C3            leave / ret
 */

#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "codegen.hpp"

namespace {

int64_t signed_bits(uint64_t value) {
    static_assert(sizeof(value) == sizeof(int64_t));
    int64_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

int64_t wrap_add(int64_t left, int64_t right) {
    return signed_bits(static_cast<uint64_t>(left)
                       + static_cast<uint64_t>(right));
}

int64_t wrap_sub(int64_t left, int64_t right) {
    return signed_bits(static_cast<uint64_t>(left)
                       - static_cast<uint64_t>(right));
}

int64_t wrap_mul(int64_t left, int64_t right) {
    return signed_bits(static_cast<uint64_t>(left)
                       * static_cast<uint64_t>(right));
}

bool contains_call(const Node &node) {
    if (node.kind == NodeKind::Call)
        return true;
    for (const NodePtr *child :
         {&node.lhs, &node.rhs, &node.then_branch, &node.else_branch}) {
        if (*child && contains_call(**child))
            return true;
    }
    for (const auto &statement : node.body)
        if (statement && contains_call(*statement))
            return true;
    for (const auto &argument : node.args)
        if (argument && contains_call(*argument))
            return true;
    return false;
}

/* ---------------------------------------------------------------- folding --
 * Constant folding is a pure AST->AST pass, so it cannot break the back end.
 * Division by zero is deliberately *not* folded: leave it to run time rather
 * than turning a runtime fault into a compile-time surprise. */
void fold(Node &node) {
    for (auto &child : {&node.lhs, &node.rhs, &node.then_branch, &node.else_branch})
        if (*child)
            fold(**child);
    for (auto &stmt : node.body)
        if (stmt)
            fold(*stmt);
    for (auto &arg : node.args)
        if (arg)
            fold(*arg);

    if (node.kind != NodeKind::Binary || !node.lhs || !node.rhs)
        return;
    if (node.lhs->kind != NodeKind::Number || node.rhs->kind != NodeKind::Number)
        return;

    const int64_t a = node.lhs->value, b = node.rhs->value;
    int64_t result;
    switch (node.op) {
    case BinOp::Add: result = wrap_add(a, b); break;
    case BinOp::Sub: result = wrap_sub(a, b); break;
    case BinOp::Mul: result = wrap_mul(a, b); break;
    case BinOp::Div:
        if (b == 0
                || (a == std::numeric_limits<int64_t>::min() && b == -1))
            return;                        // leave hardware faults for run time
        result = a / b; break;
    case BinOp::Lt: result = a <  b; break;
    case BinOp::Gt: result = a >  b; break;
    case BinOp::Le: result = a <= b; break;
    case BinOp::Ge: result = a >= b; break;
    case BinOp::Eq: result = a == b; break;
    case BinOp::Ne: result = a != b; break;
    default: return;
    }
    node.kind = NodeKind::Number;
    node.value = result;
    node.lhs.reset();
    node.rhs.reset();
}

/* System V passes the first six integer arguments in these registers. */
constexpr uint8_t ARG_REGS[6] = {7, 6, 2, 1, 8, 9}; // rdi rsi rdx rcx r8 r9

class Compiler {
public:
    explicit Compiler(bool optimise) : optimise_(optimise) {}

    CompiledProgram compile(const Node &program) {
        // Collect functions first so calls can be resolved to any of them.
        for (const auto &stmt : program.body)
            if (stmt->kind == NodeKind::Function)
                declared_.insert(stmt->name);

        for (const auto &stmt : program.body)
            if (stmt->kind == NodeKind::Function)
                compile_function(*stmt);

        // The top-level statements become an implicit entry function.
        const size_t entry = code_.size();
        Node main_fn;
        main_fn.kind = NodeKind::Function;
        main_fn.name = "@entry";
        auto block = Node::make(NodeKind::Block);
        for (const auto &stmt : program.body)
            if (stmt->kind != NodeKind::Function)
                block->body.push_back(clone(*stmt));
        main_fn.then_branch = std::move(block);
        compile_function(main_fn);

        patch_calls();
        if (optimise_)
            peephole_report_();
        return CompiledProgram{std::move(code_), entry};
    }

private:
    std::vector<uint8_t> code_;
    bool optimise_;

    std::map<std::string, size_t> function_offsets_;
    std::set<std::string> declared_;
    struct CallPatch { size_t at; std::string name; };
    std::vector<CallPatch> call_patches_;

    std::map<std::string, int> locals_;   // name -> slot (1-based)
    int next_slot_ = 0;
    size_t frame_patch_ = 0;
    std::vector<size_t> return_patches_;
    size_t peep_removed_ = 0;
    size_t temp_stack_bytes_ = 0;
    std::vector<uint8_t> free_scratch_{11, 10}; // R10, then R11

    // ---- emit helpers ----
    void b(uint8_t x) { code_.push_back(x); }
    void bs(std::initializer_list<uint8_t> xs) { for (uint8_t x : xs) b(x); }
    void imm32(int32_t v) {
        auto u = static_cast<uint32_t>(v);
        for (int i = 0; i < 4; i++) b(static_cast<uint8_t>((u >> (i * 8)) & 0xFF));
    }
    void imm64(int64_t v) {
        auto u = static_cast<uint64_t>(v);
        for (int i = 0; i < 8; i++) b(static_cast<uint8_t>((u >> (i * 8)) & 0xFF));
    }
    void imm8(int64_t v) {
        b(static_cast<uint8_t>(static_cast<int8_t>(v)));
    }
    void patch32(size_t at, int32_t v) {
        auto u = static_cast<uint32_t>(v);
        for (int i = 0; i < 4; i++) code_[at + i] = static_cast<uint8_t>((u >> (i * 8)) & 0xFF);
    }

    void push_rax() {
        b(0x50);                                // push rax
        temp_stack_bytes_ += 8;
    }

    void pop_reg(uint8_t reg) {
        if (temp_stack_bytes_ < 8)
            throw std::runtime_error("codegen: temporary stack underflow");
        if (reg >= 8) {
            b(0x41);
            b(static_cast<uint8_t>(0x58 + (reg & 7)));
        } else {
            b(static_cast<uint8_t>(0x58 + reg));
        }
        temp_stack_bytes_ -= 8;
    }

    void mov_rax_imm(int64_t v) {
        // Peephole: a value that fits in 32 bits uses the 5-byte form instead
        // of the 10-byte movabs (writing eax zero-extends into rax for free).
        if (optimise_ && v >= 0 && v <= 0x7FFFFFFF) {
            b(0xB8); imm32(static_cast<int32_t>(v));
            peep_removed_ += 5;
        } else {
            bs({0x48, 0xB8}); imm64(v);
        }
    }

    /* Emit `jcc`/`jmp` with a placeholder and return the displacement offset. */
    size_t emit_jump_placeholder(bool conditional_je) {
        if (conditional_je) bs({0x0F, 0x84});
        else                b(0xE9);
        const size_t at = code_.size();
        imm32(0);
        return at;
    }

    /* Displacements are relative to the END of the jump instruction. */
    void patch_jump_to_here(size_t at) {
        patch32(at, static_cast<int32_t>(code_.size() - (at + 4)));
    }

    void emit_jump_back(size_t target) {
        b(0xE9);
        const size_t at = code_.size();
        imm32(static_cast<int32_t>(static_cast<int64_t>(target) - static_cast<int64_t>(at + 4)));
    }

    int slot_of(const std::string &name) {
        auto it = locals_.find(name);
        if (it == locals_.end())
            throw std::runtime_error("undefined variable '" + name + "'");
        return it->second;
    }

    int declare(const std::string &name) {
        auto it = locals_.find(name);
        if (it != locals_.end())
            return it->second;          // re-binding reuses the slot
        locals_[name] = ++next_slot_;
        return next_slot_;
    }

    void store_local(int slot) {   // mov [rbp - 8*slot], rax
        bs({0x48, 0x89, 0x85});
        imm32(-8 * slot);
    }
    void load_local(int slot) {    // mov rax, [rbp - 8*slot]
        bs({0x48, 0x8B, 0x85});
        imm32(-8 * slot);
    }

    // ---- functions ----
    void compile_function(const Node &fn) {
        function_offsets_[fn.name] = code_.size();
        locals_.clear();
        next_slot_ = 0;
        return_patches_.clear();
        temp_stack_bytes_ = 0;
        free_scratch_ = {11, 10};

        bs({0x55});                       // push rbp
        bs({0x48, 0x89, 0xE5});           // mov rbp, rsp
        bs({0x48, 0x81, 0xEC});           // sub rsp, imm32
        frame_patch_ = code_.size();
        imm32(0);                         // patched once the slot count is known

        // Spill incoming parameters into locals so the rest of codegen treats
        // them exactly like any other variable.
        for (size_t i = 0; i < fn.params.size(); i++) {
            const int slot = declare(fn.params[i]);
            const uint8_t reg = ARG_REGS[i];
            // mov [rbp-8*slot], reg   (REX.W + optional REX.R for r8/r9)
            b(reg >= 8 ? 0x4C : 0x48);
            b(0x89);
            b(static_cast<uint8_t>(0x85 | ((reg & 7) << 3)));
            imm32(-8 * slot);
        }

        mov_rax_imm(0);                   // functions default to returning 0
        compile_node(*fn.then_branch);
        if (temp_stack_bytes_ != 0)
            throw std::runtime_error("codegen: unbalanced temporary stack");
        if (free_scratch_.size() != 2)
            throw std::runtime_error("codegen: leaked scratch register");

        for (size_t at : return_patches_)
            patch_jump_to_here(at);

        // Frame size: 8 per local, rounded up to 16 so RSP stays aligned at
        // every call site (misalignment crashes inside libc, not here).
        const int32_t frame = ((next_slot_ * 8) + 15) / 16 * 16;
        patch32(frame_patch_, frame);

        bs({0xC9});                       // leave
        bs({0xC3});                       // ret
    }

    void patch_calls() {
        for (const auto &patch : call_patches_) {
            auto it = function_offsets_.find(patch.name);
            if (it == function_offsets_.end())
                throw std::runtime_error("call to undefined function '" + patch.name + "'");
            patch32(patch.at,
                    static_cast<int32_t>(static_cast<int64_t>(it->second)
                                         - static_cast<int64_t>(patch.at + 4)));
        }
    }

    void peephole_report_() {}

    // ---- statements and expressions ----
    void compile_node(const Node &node) {
        switch (node.kind) {
        case NodeKind::Number:
            mov_rax_imm(node.value);
            break;

        case NodeKind::Var:
            load_local(slot_of(node.name));
            break;

        case NodeKind::Let: {
            compile_node(*node.rhs);
            store_local(declare(node.name));
            break;
        }

        case NodeKind::Assign: {
            compile_node(*node.rhs);
            store_local(slot_of(node.name));   // must already exist
            break;
        }

        case NodeKind::Block:
            for (const auto &stmt : node.body)
                compile_node(*stmt);
            break;

        case NodeKind::Binary:
            compile_binary(node);
            break;

        case NodeKind::If: {
            compile_node(*node.lhs);
            bs({0x48, 0x85, 0xC0});                 // test rax, rax
            const size_t to_else = emit_jump_placeholder(true);
            compile_node(*node.then_branch);
            if (node.else_branch) {
                const size_t to_end = emit_jump_placeholder(false);
                patch_jump_to_here(to_else);
                compile_node(*node.else_branch);
                patch_jump_to_here(to_end);
            } else {
                patch_jump_to_here(to_else);
            }
            break;
        }

        case NodeKind::While: {
            const size_t loop_start = code_.size();
            compile_node(*node.lhs);
            bs({0x48, 0x85, 0xC0});                 // test rax, rax
            const size_t to_end = emit_jump_placeholder(true);
            compile_node(*node.then_branch);
            emit_jump_back(loop_start);
            patch_jump_to_here(to_end);
            break;
        }

        case NodeKind::Return:
            compile_node(*node.rhs);
            return_patches_.push_back(emit_jump_placeholder(false));
            break;

        case NodeKind::Call:
            compile_call(node);
            break;

        case NodeKind::Function:
        case NodeKind::Program:
            throw std::runtime_error("codegen: nested functions are not supported");
        }
    }

    void compile_binary(const Node &node) {
        if (emit_binary_immediate(node))
            return;
        if (optimise_ && node.op != BinOp::Div
                && !contains_call(*node.rhs) && !free_scratch_.empty()) {
            compile_binary_register(node);
            return;
        }

        compile_node(*node.lhs);
        push_rax();
        compile_node(*node.rhs);
        bs({0x48, 0x89, 0xC1});                     // mov rcx, rax  (right)
        pop_reg(0);                                  // pop rax       (left)
        emit_binary_rax_rcx(node.op);
    }

    void emit_binary_rax_rcx(BinOp op) {
        switch (op) {
        case BinOp::Add: bs({0x48, 0x01, 0xC8}); break;
        case BinOp::Sub: bs({0x48, 0x29, 0xC8}); break;
        case BinOp::Mul: bs({0x48, 0x0F, 0xAF, 0xC1}); break;
        case BinOp::Div:
            bs({0x48, 0x99});                       // cqo — idiv needs RDX:RAX
            bs({0x48, 0xF7, 0xF9});                 // idiv rcx
            break;
        default: {
            bs({0x48, 0x39, 0xC8});                 // cmp rax, rcx
            emit_setcc(op);
            break;
        }
        }
    }

    void emit_setcc(BinOp op) {
        uint8_t opcode;
        switch (op) {
        case BinOp::Lt: opcode = 0x9C; break;        // setl
        case BinOp::Gt: opcode = 0x9F; break;        // setg
        case BinOp::Le: opcode = 0x9E; break;        // setle
        case BinOp::Ge: opcode = 0x9D; break;        // setge
        case BinOp::Eq: opcode = 0x94; break;        // sete
        case BinOp::Ne: opcode = 0x95; break;        // setne
        default:
            throw std::runtime_error("codegen: expected a comparison");
        }
        bs({0x0F, opcode, 0xC0});                    // setcc al
        bs({0x48, 0x0F, 0xB6, 0xC0});               // movzx rax, al
    }

    bool emit_binary_immediate(const Node &node) {
        if (!optimise_ || node.rhs->kind != NodeKind::Number
                || node.rhs->value < -128 || node.rhs->value > 127
                || node.op == BinOp::Div)
            return false;

        compile_node(*node.lhs);
        switch (node.op) {
        case BinOp::Add:
            bs({0x48, 0x83, 0xC0}); imm8(node.rhs->value); // add rax, imm8
            break;
        case BinOp::Sub:
            bs({0x48, 0x83, 0xE8}); imm8(node.rhs->value); // sub rax, imm8
            break;
        case BinOp::Mul:
            bs({0x48, 0x6B, 0xC0}); imm8(node.rhs->value); // imul rax, rax, imm8
            break;
        default:
            bs({0x48, 0x83, 0xF8}); imm8(node.rhs->value); // cmp rax, imm8
            emit_setcc(node.op);
            break;
        }
        return true;
    }

    void compile_binary_register(const Node &node) {
        compile_node(*node.lhs);
        const uint8_t scratch = free_scratch_.back();
        free_scratch_.pop_back();

        // 49 89 C2/C3: mov r10/r11, rax (REX.W+B, 89 /r)
        bs({0x49, 0x89, static_cast<uint8_t>(0xC0 | (scratch & 7))});
        compile_node(*node.rhs);

        switch (node.op) {
        case BinOp::Add:
            // 49 03 C2/C3: add rax, r10/r11
            bs({0x49, 0x03, static_cast<uint8_t>(0xC0 | (scratch & 7))});
            break;
        case BinOp::Mul:
            // 49 0F AF C2/C3: imul rax, r10/r11
            bs({0x49, 0x0F, 0xAF,
                static_cast<uint8_t>(0xC0 | (scratch & 7))});
            break;
        case BinOp::Sub:
            // 49 29 C2/C3: sub r10/r11, rax; 4C 89 D0/D8: mov rax, scratch
            bs({0x49, 0x29, static_cast<uint8_t>(0xC0 | (scratch & 7))});
            bs({0x4C, 0x89,
                static_cast<uint8_t>(0xC0 | ((scratch & 7) << 3))});
            break;
        default:
            // 49 39 C2/C3: cmp r10/r11, rax (left - right)
            bs({0x49, 0x39, static_cast<uint8_t>(0xC0 | (scratch & 7))});
            emit_setcc(node.op);
            break;
        }

        free_scratch_.push_back(scratch);
    }

    void compile_call(const Node &node) {
        if (!declared_.count(node.name))
            throw std::runtime_error("call to undefined function '" + node.name + "'");

        // Evaluate arguments left to right, stacking each result, then pop them
        // into the argument registers. Going through the stack keeps a nested
        // call from clobbering an argument register already loaded.
        for (const auto &arg : node.args) {
            compile_node(*arg);
            push_rax();
        }
        for (size_t i = node.args.size(); i-- > 0;) {
            const uint8_t reg = ARG_REGS[i];
            pop_reg(reg);
        }

        // Expression temporaries may have shifted RSP by eight bytes. System V
        // requires 16-byte alignment immediately before CALL.
        const bool needs_padding = temp_stack_bytes_ % 16 != 0;
        if (needs_padding) {
            bs({0x48, 0x83, 0xEC, 0x08});           // sub rsp, 8
            temp_stack_bytes_ += 8;
        }
        b(0xE8);                                    // call rel32
        call_patches_.push_back({code_.size(), node.name});
        imm32(0);
        if (needs_padding) {
            bs({0x48, 0x83, 0xC4, 0x08});           // add rsp, 8
            temp_stack_bytes_ -= 8;
        }
    }

    static NodePtr clone(const Node &node) {
        auto copy = Node::make(node.kind, node.pos);
        copy->value = node.value;
        copy->name = node.name;
        copy->op = node.op;
        copy->params = node.params;
        if (node.lhs)          copy->lhs = clone(*node.lhs);
        if (node.rhs)          copy->rhs = clone(*node.rhs);
        if (node.then_branch)  copy->then_branch = clone(*node.then_branch);
        if (node.else_branch)  copy->else_branch = clone(*node.else_branch);
        for (const auto &s : node.body) copy->body.push_back(clone(*s));
        for (const auto &a : node.args) copy->args.push_back(clone(*a));
        return copy;
    }
};

} // namespace

CompiledProgram codegen(const Node &program, bool optimise) {
    if (optimise) {
        // fold() mutates, so work on a copy of the tree the caller owns.
        Node copy;
        copy.kind = program.kind;
        for (const auto &stmt : program.body) {
            struct Cloner {
                static NodePtr run(const Node &n) {
                    auto c = Node::make(n.kind, n.pos);
                    c->value = n.value; c->name = n.name; c->op = n.op;
                    c->params = n.params;
                    if (n.lhs) c->lhs = run(*n.lhs);
                    if (n.rhs) c->rhs = run(*n.rhs);
                    if (n.then_branch) c->then_branch = run(*n.then_branch);
                    if (n.else_branch) c->else_branch = run(*n.else_branch);
                    for (const auto &s : n.body) c->body.push_back(run(*s));
                    for (const auto &a : n.args) c->args.push_back(run(*a));
                    return c;
                }
            };
            copy.body.push_back(Cloner::run(*stmt));
        }
        for (auto &stmt : copy.body)
            fold(*stmt);
        Compiler compiler(true);
        return compiler.compile(copy);
    }
    Compiler compiler(false);
    return compiler.compile(program);
}
