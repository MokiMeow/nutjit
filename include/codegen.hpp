#pragma once
#include <cstdint>
#include <vector>

#include "ast.hpp"

/* Stage 3: AST -> x86-64 machine code.
 *
 * Emits a complete System V AMD64 function: it takes no arguments and returns
 * the expression's value in RAX, so the host can call it as `int64_t(*)()`. */
std::vector<uint8_t> codegen(const Node &root);
