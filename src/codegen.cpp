/* Stage 3: AST -> raw x86-64 machine code bytes.
 *
 * Strategy: a stack machine. Every expression leaves its result in RAX. For a
 * binary node we compile the left side, push RAX, compile the right side, move
 * it to RCX, pop the left back into RAX, and emit one instruction. That keeps
 * codegen to a few lines per operator and needs no register allocator — which
 * is milestone 5's job.
 *
 * Encodings used (Intel manual, 64-bit operand size via the REX.W prefix 0x48):
 *   48 B8 imm64        movabs rax, imm64
 *   50 / 58 / 59       push rax / pop rax / pop rcx
 *   48 89 C1           mov rcx, rax
 *   48 01 C8           add rax, rcx
 *   48 29 C8           sub rax, rcx
 *   48 0F AF C1        imul rax, rcx
 *   48 99              cqo            (sign-extend RAX into RDX:RAX)
 *   48 F7 F9           idiv rcx
 *   C3                 ret
 */

#include <stdexcept>

#include "codegen.hpp"

namespace {

class Emitter {
public:
    void byte(uint8_t b) { code_.push_back(b); }

    void bytes(std::initializer_list<uint8_t> bs) {
        for (uint8_t b : bs)
            code_.push_back(b);
    }

    /* movabs rax, imm64 — the full 64-bit form, so large literals work. */
    void mov_rax_imm64(int64_t value) {
        bytes({0x48, 0xB8});
        auto raw = static_cast<uint64_t>(value);
        for (int i = 0; i < 8; i++)
            byte(static_cast<uint8_t>((raw >> (i * 8)) & 0xFF));
    }

    void emit(const Node &node) {
        if (node.kind == NodeKind::Number) {
            mov_rax_imm64(node.value);
            return;
        }

        emit(*node.lhs);          /* left  -> rax           */
        byte(0x50);               /* push rax               */
        emit(*node.rhs);          /* right -> rax           */
        bytes({0x48, 0x89, 0xC1}); /* mov rcx, rax  (right)  */
        byte(0x58);               /* pop rax       (left)   */

        switch (node.op) {
        case BinOp::Add: bytes({0x48, 0x01, 0xC8}); break;
        case BinOp::Sub: bytes({0x48, 0x29, 0xC8}); break;
        case BinOp::Mul: bytes({0x48, 0x0F, 0xAF, 0xC1}); break;
        case BinOp::Div:
            /* cqo sign-extends RAX into RDX:RAX, which idiv requires. */
            bytes({0x48, 0x99});
            bytes({0x48, 0xF7, 0xF9});
            break;
        default:
            throw std::runtime_error("codegen: unknown binary operator");
        }
    }

    std::vector<uint8_t> finish() {
        byte(0xC3); /* ret */
        return std::move(code_);
    }

private:
    std::vector<uint8_t> code_;
};

} // namespace

std::vector<uint8_t> codegen(const Node &root) {
    Emitter emitter;
    emitter.emit(root);
    return emitter.finish();
}
