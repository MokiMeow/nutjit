/* The reference interpreter: the oracle and the benchmark baseline.
 *
 * Deliberately the most obvious implementation possible — recursive evaluation
 * over a map-based environment. Its job is to be *clearly correct*, not fast.
 * When the JIT and this disagree, the JIT is wrong. */

#include <cstring>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "interp.hpp"

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

/* Thrown to unwind out of a `return`. */
struct ReturnValue {
    int64_t value;
};

using Env = std::map<std::string, int64_t>;

class Interpreter {
public:
    explicit Interpreter(const Node &program) {
        for (const auto &stmt : program.body)
            if (stmt->kind == NodeKind::Function)
                functions_[stmt->name] = stmt.get();
    }

    int64_t run(const Node &program) {
        Env env;
        int64_t last = 0;
        for (const auto &stmt : program.body)
            if (stmt->kind != NodeKind::Function)
                last = eval(*stmt, env);
        return last;
    }

private:
    std::map<std::string, const Node *> functions_;
    int depth_ = 0;

    int64_t eval(const Node &node, Env &env) {
        switch (node.kind) {
        case NodeKind::Number:
            return node.value;

        case NodeKind::Var: {
            auto it = env.find(node.name);
            if (it == env.end())
                throw std::runtime_error("undefined variable '" + node.name + "'");
            return it->second;
        }

        case NodeKind::Let: {
            const int64_t value = eval(*node.rhs, env);
            env[node.name] = value;
            return value;
        }

        case NodeKind::Assign: {
            if (!env.count(node.name))
                throw std::runtime_error("undefined variable '" + node.name + "'");
            const int64_t value = eval(*node.rhs, env);
            env[node.name] = value;
            return value;
        }

        case NodeKind::Binary: {
            const int64_t a = eval(*node.lhs, env);
            const int64_t b = eval(*node.rhs, env);
            switch (node.op) {
            case BinOp::Add: return wrap_add(a, b);
            case BinOp::Sub: return wrap_sub(a, b);
            case BinOp::Mul: return wrap_mul(a, b);
            case BinOp::Div:
                if (b == 0)
                    throw std::runtime_error("division by zero");
                if (a == std::numeric_limits<int64_t>::min() && b == -1)
                    throw std::runtime_error("division overflow");
                return a / b;
            case BinOp::Lt: return a <  b;
            case BinOp::Gt: return a >  b;
            case BinOp::Le: return a <= b;
            case BinOp::Ge: return a >= b;
            case BinOp::Eq: return a == b;
            case BinOp::Ne: return a != b;
            }
            throw std::runtime_error("unknown operator");
        }

        case NodeKind::Block: {
            int64_t last = 0;
            for (const auto &stmt : node.body)
                last = eval(*stmt, env);
            return last;
        }

        case NodeKind::If:
            if (eval(*node.lhs, env) != 0)
                return eval(*node.then_branch, env);
            if (node.else_branch)
                return eval(*node.else_branch, env);
            return 0;

        case NodeKind::While: {
            int64_t last = 0;
            while (eval(*node.lhs, env) != 0)
                last = eval(*node.then_branch, env);
            return last;
        }

        case NodeKind::Return:
            throw ReturnValue{eval(*node.rhs, env)};

        case NodeKind::Call: {
            auto it = functions_.find(node.name);
            if (it == functions_.end())
                throw std::runtime_error("call to undefined function '" + node.name + "'");
            const Node &fn = *it->second;
            if (node.args.size() != fn.params.size())
                throw std::runtime_error("wrong number of arguments to '" + node.name + "'");

            // Evaluate arguments in the caller's environment first.
            std::vector<int64_t> values;
            values.reserve(node.args.size());
            for (const auto &arg : node.args)
                values.push_back(eval(*arg, env));

            if (++depth_ > 10000) {
                depth_--;
                throw std::runtime_error("recursion too deep");
            }
            Env local;
            for (size_t i = 0; i < fn.params.size(); i++)
                local[fn.params[i]] = values[i];

            int64_t result = 0;
            try {
                result = eval(*fn.then_branch, local);
            } catch (const ReturnValue &r) {
                result = r.value;
            }
            depth_--;
            return result;
        }

        case NodeKind::Function:
            return 0;   // declarations produce no value

        case NodeKind::Program:
            throw std::runtime_error("nested program node");
        }
        throw std::runtime_error("unhandled node kind");
    }
};

} // namespace

int64_t interpret(const Node &program) {
    Interpreter interpreter(program);
    return interpreter.run(program);
}
