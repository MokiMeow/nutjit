/* Stage 1 of the pipeline: source text -> tokens.
 *
 * A hand-written scanner, not a generated one: the whole point of the project
 * is that every stage is ours and readable. */

#include <cctype>
#include <stdexcept>
#include <string>

#include "lexer.hpp"

std::vector<Token> tokenize(const std::string &source) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < source.size()) {
        const char c = source[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            const size_t start = i;
            int64_t value = 0;
            while (i < source.size()
                   && std::isdigit(static_cast<unsigned char>(source[i]))) {
                value = value * 10 + (source[i] - '0');
                i++;
            }
            tokens.push_back({TokKind::Number, value, start});
            continue;
        }

        TokKind kind;
        switch (c) {
        case '+': kind = TokKind::Plus;   break;
        case '-': kind = TokKind::Minus;  break;
        case '*': kind = TokKind::Star;   break;
        case '/': kind = TokKind::Slash;  break;
        case '(': kind = TokKind::LParen; break;
        case ')': kind = TokKind::RParen; break;
        default:
            throw std::runtime_error("unexpected character '" + std::string(1, c)
                                     + "' at offset " + std::to_string(i));
        }
        tokens.push_back({kind, 0, i});
        i++;
    }

    tokens.push_back({TokKind::End, 0, i});
    return tokens;
}
