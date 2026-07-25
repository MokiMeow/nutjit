#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class TokKind {
    Number, Ident,
    Let, If, Else, While, Fn, Return,          // keywords
    Plus, Minus, Star, Slash,
    Lt, Gt, Le, Ge, EqEq, NotEq, Assign,
    LParen, RParen, LBrace, RBrace, Comma, Semicolon,
    End,
};

struct Token {
    TokKind     kind;
    int64_t     value = 0;   // Number
    std::string text;        // Ident
    size_t      pos = 0;     // byte offset, for error messages
};

/* Turn source text into a token stream. Throws std::runtime_error with the
 * offset on an unexpected character. */
std::vector<Token> tokenize(const std::string &source);
