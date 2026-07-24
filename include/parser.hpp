#pragma once
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

/* Recursive-descent parser. Grammar (milestone 0):
 *
 *   expr   := term (('+' | '-') term)*
 *   term   := factor (('*' | '/') factor)*
 *   factor := NUMBER | '(' expr ')' | '-' factor
 *
 * Precedence falls out of the shape of the grammar: `term` binds tighter than
 * `expr`, so 2+3*4 parses as 2+(3*4). Throws std::runtime_error on a syntax
 * error, reporting the offset. */
NodePtr parse(const std::vector<Token> &tokens);
