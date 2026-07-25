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

#include <algorithm>
#include <stdexcept>
#include <map>
#include <set>
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
        const size_t pos = peek().pos;
        expect(TokKind::Fn, "expected 'fn'");
        if (peek().kind != TokKind::Ident)
            throw std::runtime_error("syntax error: expected a function name at offset "
                                     + std::to_string(peek().pos));
        auto fn = Node::make(NodeKind::Function, pos);
        fn->name = advance().text;

        expect(TokKind::LParen, "expected '(' after the function name");
        while (peek().kind != TokKind::RParen) {
            if (peek().kind != TokKind::Ident)
                throw std::runtime_error("syntax error: expected a parameter name at offset "
                                         + std::to_string(peek().pos));
            const Token &param_token = advance();
            const std::string param = param_token.text;
            if (std::find(fn->params.begin(), fn->params.end(), param)
                    != fn->params.end()) {
                throw std::runtime_error(
                    "duplicate parameter '" + param + "' at offset "
                    + std::to_string(param_token.pos));
            }
            fn->params.push_back(param);
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
        const size_t pos = peek().pos;
        expect(TokKind::LBrace, "expected '{'");
        auto block = Node::make(NodeKind::Block, pos);
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
            const size_t pos = peek().pos;
            advance();
            if (peek().kind != TokKind::Ident)
                throw std::runtime_error("syntax error: expected a name after 'let' at offset "
                                         + std::to_string(peek().pos));
            auto let = Node::make(NodeKind::Let, pos);
            let->name = advance().text;
            expect(TokKind::Assign, "expected '=' in a let binding");
            let->rhs = parse_expr();
            expect(TokKind::Semicolon, "expected ';'");
            return let;
        }

        if (peek().kind == TokKind::If) {
            const size_t pos = peek().pos;
            advance();
            auto node = Node::make(NodeKind::If, pos);
            expect(TokKind::LParen, "expected '(' after 'if'");
            node->lhs = parse_expr();
            expect(TokKind::RParen, "expected ')'");
            node->then_branch = parse_block();
            if (match(TokKind::Else))
                node->else_branch = parse_block();
            return node;
        }

        if (peek().kind == TokKind::While) {
            const size_t pos = peek().pos;
            advance();
            auto node = Node::make(NodeKind::While, pos);
            expect(TokKind::LParen, "expected '(' after 'while'");
            node->lhs = parse_expr();
            expect(TokKind::RParen, "expected ')'");
            node->then_branch = parse_block();
            return node;
        }

        if (peek().kind == TokKind::Return) {
            const size_t pos = peek().pos;
            advance();
            auto node = Node::make(NodeKind::Return, pos);
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
            auto node = Node::make(NodeKind::Assign, peek().pos);
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
            const size_t pos = peek().pos;
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
            node = Node::binary(op, std::move(node), parse_sum(), pos);
        }
    }

    NodePtr parse_sum() {
        NodePtr node = parse_term();
        for (;;) {
            if (peek().kind == TokKind::Plus) {
                const size_t pos = advance().pos;
                node = Node::binary(BinOp::Add, std::move(node), parse_term(), pos);
            } else if (peek().kind == TokKind::Minus) {
                const size_t pos = advance().pos;
                node = Node::binary(BinOp::Sub, std::move(node), parse_term(), pos);
            }
            else
                return node;
        }
    }

    NodePtr parse_term() {
        NodePtr node = parse_factor();
        for (;;) {
            if (peek().kind == TokKind::Star) {
                const size_t pos = advance().pos;
                node = Node::binary(BinOp::Mul, std::move(node), parse_factor(), pos);
            } else if (peek().kind == TokKind::Slash) {
                const size_t pos = advance().pos;
                node = Node::binary(BinOp::Div, std::move(node), parse_factor(), pos);
            }
            else
                return node;
        }
    }

    NodePtr parse_factor() {
        if (peek().kind == TokKind::Minus) {
            const size_t pos = advance().pos;
            // Unary minus as 0 - x keeps the back ends to one code path.
            return Node::binary(BinOp::Sub, Node::number(0, pos),
                                parse_factor(), pos);
        }

        if (peek().kind == TokKind::Number) {
            const Token &token = advance();
            return Node::number(token.value, token.pos);
        }

        if (peek().kind == TokKind::Ident) {
            const Token &ident = advance();
            std::string name = ident.text;
            if (match(TokKind::LParen)) {
                auto call = Node::make(NodeKind::Call, ident.pos);
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
            return Node::var(std::move(name), ident.pos);
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

using Names = std::set<std::string>;
using Signatures = std::map<std::string, size_t>;

[[noreturn]] void semantic_error(const Node &node, const std::string &message) {
    throw std::runtime_error(message + " at offset " + std::to_string(node.pos));
}

void validate_node(const Node &node, Names &defined,
                   const Signatures &signatures, bool in_function) {
    switch (node.kind) {
    case NodeKind::Number:
        return;

    case NodeKind::Var:
        if (!defined.count(node.name))
            semantic_error(node, "undefined variable '" + node.name + "'");
        return;

    case NodeKind::Let:
        validate_node(*node.rhs, defined, signatures, in_function);
        defined.insert(node.name);
        return;

    case NodeKind::Assign:
        if (!defined.count(node.name))
            semantic_error(node, "undefined variable '" + node.name + "'");
        validate_node(*node.rhs, defined, signatures, in_function);
        return;

    case NodeKind::Binary:
        validate_node(*node.lhs, defined, signatures, in_function);
        validate_node(*node.rhs, defined, signatures, in_function);
        return;

    case NodeKind::Block:
        for (const auto &statement : node.body)
            validate_node(*statement, defined, signatures, in_function);
        return;

    case NodeKind::If: {
        validate_node(*node.lhs, defined, signatures, in_function);
        Names then_defined = defined;
        validate_node(*node.then_branch, then_defined, signatures, in_function);
        if (!node.else_branch)
            return;

        Names else_defined = defined;
        validate_node(*node.else_branch, else_defined, signatures, in_function);
        for (const auto &name : then_defined)
            if (else_defined.count(name))
                defined.insert(name);
        return;
    }

    case NodeKind::While: {
        validate_node(*node.lhs, defined, signatures, in_function);
        Names loop_defined = defined;
        validate_node(*node.then_branch, loop_defined, signatures, in_function);
        return;
    }

    case NodeKind::Return:
        if (!in_function)
            semantic_error(node, "'return' is only valid inside a function");
        validate_node(*node.rhs, defined, signatures, in_function);
        return;

    case NodeKind::Call: {
        auto function = signatures.find(node.name);
        if (function == signatures.end())
            semantic_error(node, "call to undefined function '" + node.name + "'");
        if (node.args.size() != function->second) {
            semantic_error(node,
                           "wrong number of arguments to '" + node.name
                           + "' (expected " + std::to_string(function->second)
                           + ", got " + std::to_string(node.args.size()) + ")");
        }
        for (const auto &argument : node.args)
            validate_node(*argument, defined, signatures, in_function);
        return;
    }

    case NodeKind::Function:
    case NodeKind::Program:
        semantic_error(node, "nested declaration");
    }
}

void validate_program(const Node &program) {
    Signatures signatures;

    for (const auto &statement : program.body) {
        if (statement->kind != NodeKind::Function)
            continue;
        if (!signatures.emplace(statement->name, statement->params.size()).second)
            semantic_error(*statement,
                           "duplicate function '" + statement->name + "'");
    }

    for (const auto &statement : program.body) {
        if (statement->kind != NodeKind::Function)
            continue;
        Names defined(statement->params.begin(), statement->params.end());
        validate_node(*statement->then_branch, defined, signatures, true);
    }

    Names defined;
    for (const auto &statement : program.body) {
        if (statement->kind != NodeKind::Function)
            validate_node(*statement, defined, signatures, false);
    }
}

} // namespace

NodePtr parse(const std::vector<Token> &tokens) {
    Parser parser(tokens);
    NodePtr program = parser.parse_program();
    validate_program(*program);
    return program;
}
