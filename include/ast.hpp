#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* The AST the parser produces and both back ends (the JIT code generator and
 * the reference interpreter) walk. */

enum class NodeKind {
    Number,   // 42
    Var,      // x
    Binary,   // a + b, a < b
    Assign,   // x = expr
    Let,      // let x = expr;
    If,       // if (cond) { .. } else { .. }
    While,    // while (cond) { .. }
    Block,    // { stmt* }
    Call,     // f(a, b)
    Return,   // return expr;
    Function, // fn f(a, b) { .. }
    Program,  // functions + top-level statements
};

enum class BinOp { Add, Sub, Mul, Div, Lt, Gt, Le, Ge, Eq, Ne };

inline bool is_comparison(BinOp op) {
    return op == BinOp::Lt || op == BinOp::Gt || op == BinOp::Le
        || op == BinOp::Ge || op == BinOp::Eq || op == BinOp::Ne;
}

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;

    int64_t     value = 0;   // Number
    std::string name;        // Var, Assign, Let, Call, Function
    BinOp       op{};        // Binary

    NodePtr lhs, rhs;        // Binary; Assign/Let/Return use rhs; If/While use lhs as the condition
    NodePtr then_branch;     // If, While body
    NodePtr else_branch;     // If

    std::vector<NodePtr>     body;    // Block / Program / Function body statements
    std::vector<NodePtr>     args;    // Call arguments
    std::vector<std::string> params;  // Function parameters

    static NodePtr make(NodeKind kind) {
        auto n = std::make_unique<Node>();
        n->kind = kind;
        return n;
    }

    static NodePtr number(int64_t v) {
        auto n = make(NodeKind::Number);
        n->value = v;
        return n;
    }

    static NodePtr var(std::string name) {
        auto n = make(NodeKind::Var);
        n->name = std::move(name);
        return n;
    }

    static NodePtr binary(BinOp op, NodePtr lhs, NodePtr rhs) {
        auto n = make(NodeKind::Binary);
        n->op = op;
        n->lhs = std::move(lhs);
        n->rhs = std::move(rhs);
        return n;
    }
};
