/* Stage 1 of the pipeline: source text -> tokens.
 *
 * A hand-written scanner. Identifiers are scanned first and then looked up in a
 * keyword table, which is the standard trick that avoids a separate keyword
 * pass and makes `lettuce` an identifier rather than `let` + `tuce`. */

#include <cctype>
#include <limits>
#include <stdexcept>
#include <string>

#include "lexer.hpp"

namespace {

TokKind keyword_or_ident(const std::string &word) {
    if (word == "let")    return TokKind::Let;
    if (word == "if")     return TokKind::If;
    if (word == "else")   return TokKind::Else;
    if (word == "while")  return TokKind::While;
    if (word == "fn")     return TokKind::Fn;
    if (word == "return") return TokKind::Return;
    return TokKind::Ident;
}

bool ident_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool ident_part(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

} // namespace

std::vector<Token> tokenize(const std::string &source) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < source.size()) {
        const char c = source[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        // Line comments.
        if (c == '/' && i + 1 < source.size() && source[i + 1] == '/') {
            while (i < source.size() && source[i] != '\n')
                i++;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            const size_t start = i;
            int64_t value = 0;
            while (i < source.size()
                   && std::isdigit(static_cast<unsigned char>(source[i]))) {
                const int digit = source[i] - '0';
                if (value > (std::numeric_limits<int64_t>::max() - digit) / 10) {
                    throw std::runtime_error(
                        "integer literal out of range at offset "
                        + std::to_string(start));
                }
                value = value * 10 + digit;
                i++;
            }
            Token t{TokKind::Number, value, {}, start};
            tokens.push_back(t);
            continue;
        }

        if (ident_start(c)) {
            const size_t start = i;
            while (i < source.size() && ident_part(source[i]))
                i++;
            std::string word = source.substr(start, i - start);
            Token t{keyword_or_ident(word), 0, word, start};
            tokens.push_back(t);
            continue;
        }

        // Two-character operators must be tried before their one-char prefixes.
        const size_t start = i;
        auto two = [&](char a, char b) {
            return c == a && i + 1 < source.size() && source[i + 1] == b;
        };

        TokKind kind;
        if (two('<', '=')) { kind = TokKind::Le;    i += 2; }
        else if (two('>', '=')) { kind = TokKind::Ge;    i += 2; }
        else if (two('=', '=')) { kind = TokKind::EqEq;  i += 2; }
        else if (two('!', '=')) { kind = TokKind::NotEq; i += 2; }
        else {
            switch (c) {
            case '+': kind = TokKind::Plus;      break;
            case '-': kind = TokKind::Minus;     break;
            case '*': kind = TokKind::Star;      break;
            case '/': kind = TokKind::Slash;     break;
            case '<': kind = TokKind::Lt;        break;
            case '>': kind = TokKind::Gt;        break;
            case '=': kind = TokKind::Assign;    break;
            case '(': kind = TokKind::LParen;    break;
            case ')': kind = TokKind::RParen;    break;
            case '{': kind = TokKind::LBrace;    break;
            case '}': kind = TokKind::RBrace;    break;
            case ',': kind = TokKind::Comma;     break;
            case ';': kind = TokKind::Semicolon; break;
            default:
                throw std::runtime_error("unexpected character '" + std::string(1, c)
                                         + "' at offset " + std::to_string(i));
            }
            i += 1;
        }
        Token t{kind, 0, {}, start};
        tokens.push_back(t);
    }

    tokens.push_back({TokKind::End, 0, {}, i});
    return tokens;
}
