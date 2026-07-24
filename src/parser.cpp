/* Stage 2: tokens -> AST.
 *
 * Recursive descent, one function per precedence level. This is the classic
 * structure: each level consumes the level below it and then loops over its
 * own operators, which is what encodes precedence and left-associativity. */

#include <stdexcept>
#include <string>
#include <utility>

#include "parser.hpp"

namespace {

class Parser {
public:
    explicit Parser(const std::vector<Token> &tokens) : tokens_(tokens) {}

    NodePtr parse_program() {
        NodePtr node = parse_expr();
        expect(TokKind::End, "trailing input");
        return node;
    }

private:
    const std::vector<Token> &tokens_;
    size_t index_ = 0;

    const Token &peek() const { return tokens_[index_]; }
    const Token &advance() { return tokens_[index_++]; }

    bool match(TokKind kind) {
        if (peek().kind != kind)
            return false;
        index_++;
        return true;
    }

    void expect(TokKind kind, const char *what) {
        if (peek().kind != kind)
            throw std::runtime_error(std::string("syntax error: ") + what
                                     + " at offset " + std::to_string(peek().pos));
    }

    /* expr := term (('+' | '-') term)* */
    NodePtr parse_expr() {
        NodePtr node = parse_term();
        for (;;) {
            if (match(TokKind::Plus))
                node = Node::binary(BinOp::Add, std::move(node), parse_term());
            else if (match(TokKind::Minus))
                node = Node::binary(BinOp::Sub, std::move(node), parse_term());
            else
                return node;
        }
    }

    /* term := factor (('*' | '/') factor)* */
    NodePtr parse_term() {
        NodePtr node = parse_factor();
        for (;;) {
            if (match(TokKind::Star))
                node = Node::binary(BinOp::Mul, std::move(node), parse_factor());
            else if (match(TokKind::Slash))
                node = Node::binary(BinOp::Div, std::move(node), parse_factor());
            else
                return node;
        }
    }

    /* factor := NUMBER | '(' expr ')' | '-' factor */
    NodePtr parse_factor() {
        if (match(TokKind::Minus))
            /* Unary minus as 0 - x keeps the code generator to one code path. */
            return Node::binary(BinOp::Sub, Node::number(0), parse_factor());

        if (peek().kind == TokKind::Number)
            return Node::number(advance().value);

        if (match(TokKind::LParen)) {
            NodePtr node = parse_expr();
            expect(TokKind::RParen, "expected ')'");
            advance();
            return node;
        }

        throw std::runtime_error("syntax error: expected a number or '(' at offset "
                                 + std::to_string(peek().pos));
    }
};

} // namespace

NodePtr parse(const std::vector<Token> &tokens) {
    Parser parser(tokens);
    return parser.parse_program();
}
