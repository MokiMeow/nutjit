/* Stage 2: tokens -> AST.
 *
 * Recursive descent, one function per precedence level:
 *
 *   program    := (function | statement)*
 *   function   := 'fn' IDENT '(' params ')' block
 *   statement  := 'let' IDENT '=' expr ';'
 *               | 'if' '(' expr ')' block ('else' block)?
 *               | 'while' '(' expr ')' block
 *               | 'return' expr ';'
 *               | block
 *               | expr ';'
 *   expr       := assignment
 *   assignment := IDENT '=' assignment | comparison
 *   comparison := sum (('<'|'>'|'<='|'>='|'=='|'!=') sum)*
 *   sum        := term (('+'|'-') term)*
 *   term       := factor (('*'|'/') factor)*
 *   factor     := NUMBER | IDENT | IDENT '(' args ')' | '(' expr ')' | '-' factor
 *
 * Precedence falls out of the nesting; left associativity falls out of the
 * loops. */

#include <stdexcept>
#include <string>
#include <utility>

#include "parser.hpp"

namespace {

class Parser {
public:
    explicit Parser(const std::vector<Token> &tokens) : tokens_(tokens) {}

    NodePtr parse_program() {
        auto program = Node::make(NodeKind::Program);
        while (peek().kind != TokKind::End) {
            if (peek().kind == TokKind::Fn)
                program->body.push_back(parse_function());
            else
                program->body.push_back(parse_statement());
        }
        return program;
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
        index_++;
    }

    NodePtr parse_function() {
        expect(TokKind::Fn, "expected 'fn'");
        if (peek().kind != TokKind::Ident)
            throw std::runtime_error("syntax error: expected a function name at offset "
                                     + std::to_string(peek().pos));
        auto fn = Node::make(NodeKind::Function);
        fn->name = advance().text;

        expect(TokKind::LParen, "expected '(' after the function name");
        while (peek().kind != TokKind::RParen) {
            if (peek().kind != TokKind::Ident)
                throw std::runtime_error("syntax error: expected a parameter name at offset "
                                         + std::to_string(peek().pos));
            fn->params.push_back(advance().text);
            if (!match(TokKind::Comma))
                break;
        }
        expect(TokKind::RParen, "expected ')' after the parameters");

        if (fn->params.size() > 6)
            throw std::runtime_error("functions take at most 6 parameters (System V "
                                     "passes the first six in registers)");

        fn->then_branch = parse_block();
        return fn;
    }

    NodePtr parse_block() {
        expect(TokKind::LBrace, "expected '{'");
        auto block = Node::make(NodeKind::Block);
        while (peek().kind != TokKind::RBrace) {
            if (peek().kind == TokKind::End)
                throw std::runtime_error("syntax error: unclosed '{'");
            block->body.push_back(parse_statement());
        }
        expect(TokKind::RBrace, "expected '}'");
        return block;
    }

    NodePtr parse_statement() {
        if (peek().kind == TokKind::Let) {
            advance();
            if (peek().kind != TokKind::Ident)
                throw std::runtime_error("syntax error: expected a name after 'let' at offset "
                                         + std::to_string(peek().pos));
            auto let = Node::make(NodeKind::Let);
            let->name = advance().text;
            expect(TokKind::Assign, "expected '=' in a let binding");
            let->rhs = parse_expr();
            expect(TokKind::Semicolon, "expected ';'");
            return let;
        }

        if (peek().kind == TokKind::If) {
            advance();
            auto node = Node::make(NodeKind::If);
            expect(TokKind::LParen, "expected '(' after 'if'");
            node->lhs = parse_expr();
            expect(TokKind::RParen, "expected ')'");
            node->then_branch = parse_block();
            if (match(TokKind::Else))
                node->else_branch = parse_block();
            return node;
        }

        if (peek().kind == TokKind::While) {
            advance();
            auto node = Node::make(NodeKind::While);
            expect(TokKind::LParen, "expected '(' after 'while'");
            node->lhs = parse_expr();
            expect(TokKind::RParen, "expected ')'");
            node->then_branch = parse_block();
            return node;
        }

        if (peek().kind == TokKind::Return) {
            advance();
            auto node = Node::make(NodeKind::Return);
            node->rhs = parse_expr();
            expect(TokKind::Semicolon, "expected ';'");
            return node;
        }

        if (peek().kind == TokKind::LBrace)
            return parse_block();

        NodePtr expr = parse_expr();
        expect(TokKind::Semicolon, "expected ';'");
        return expr;
    }

    NodePtr parse_expr() { return parse_assignment(); }

    NodePtr parse_assignment() {
        // Only `IDENT = expr` is assignable, so a one-token lookahead decides.
        if (peek().kind == TokKind::Ident
                && tokens_[index_ + 1].kind == TokKind::Assign) {
            auto node = Node::make(NodeKind::Assign);
            node->name = advance().text;
            advance(); // '='
            node->rhs = parse_assignment();
            return node;
        }
        return parse_comparison();
    }

    NodePtr parse_comparison() {
        NodePtr node = parse_sum();
        for (;;) {
            BinOp op;
            switch (peek().kind) {
            case TokKind::Lt:    op = BinOp::Lt; break;
            case TokKind::Gt:    op = BinOp::Gt; break;
            case TokKind::Le:    op = BinOp::Le; break;
            case TokKind::Ge:    op = BinOp::Ge; break;
            case TokKind::EqEq:  op = BinOp::Eq; break;
            case TokKind::NotEq: op = BinOp::Ne; break;
            default: return node;
            }
            advance();
            node = Node::binary(op, std::move(node), parse_sum());
        }
    }

    NodePtr parse_sum() {
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

    NodePtr parse_factor() {
        if (match(TokKind::Minus))
            // Unary minus as 0 - x keeps the back ends to one code path.
            return Node::binary(BinOp::Sub, Node::number(0), parse_factor());

        if (peek().kind == TokKind::Number)
            return Node::number(advance().value);

        if (peek().kind == TokKind::Ident) {
            std::string name = advance().text;
            if (match(TokKind::LParen)) {
                auto call = Node::make(NodeKind::Call);
                call->name = std::move(name);
                while (peek().kind != TokKind::RParen) {
                    call->args.push_back(parse_expr());
                    if (!match(TokKind::Comma))
                        break;
                }
                expect(TokKind::RParen, "expected ')' after the arguments");
                if (call->args.size() > 6)
                    throw std::runtime_error("calls take at most 6 arguments");
                return call;
            }
            return Node::var(std::move(name));
        }

        if (match(TokKind::LParen)) {
            NodePtr node = parse_expr();
            expect(TokKind::RParen, "expected ')'");
            return node;
        }

        throw std::runtime_error("syntax error: expected an expression at offset "
                                 + std::to_string(peek().pos));
    }
};

} // namespace

NodePtr parse(const std::vector<Token> &tokens) {
    Parser parser(tokens);
    return parser.parse_program();
}
