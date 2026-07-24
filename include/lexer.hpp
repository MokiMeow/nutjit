#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class TokKind { Number, Plus, Minus, Star, Slash, LParen, RParen, End };

struct Token {
    TokKind kind;
    int64_t value = 0; /* Number only */
    size_t  pos = 0;   /* byte offset in the source, for error messages */
};

/* Turns source text into a token stream. Throws std::runtime_error on an
 * unexpected character, reporting the offset. */
std::vector<Token> tokenize(const std::string &source);
