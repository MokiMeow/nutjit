#pragma once
#include <cstdint>
#include <memory>
#include <utility>

/* The AST the parser produces and the code generator walks.
 * Milestone 0 needs only integer literals and binary arithmetic; later
 * milestones add variables, comparisons, control flow, and calls. */

enum class NodeKind { Number, Binary };
enum class BinOp { Add, Sub, Mul, Div };

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;
    int64_t value = 0;   /* Number */
    BinOp op{};          /* Binary */
    NodePtr lhs, rhs;    /* Binary */

    static NodePtr number(int64_t v) {
        auto n = std::make_unique<Node>();
        n->kind = NodeKind::Number;
        n->value = v;
        return n;
    }

    static NodePtr binary(BinOp op, NodePtr lhs, NodePtr rhs) {
        auto n = std::make_unique<Node>();
        n->kind = NodeKind::Binary;
        n->op = op;
        n->lhs = std::move(lhs);
        n->rhs = std::move(rhs);
        return n;
    }
};
