#pragma once
#include <cstdint>

#include "ast.hpp"

/* A tree-walking interpreter over the same AST.
 *
 * It exists for two reasons, neither of which is running code in production:
 *
 *   1. It is the **oracle**. Every test runs a program through both back ends
 *      and asserts they agree — a codegen bug that produces a plausible wrong
 *      number is otherwise nearly impossible to catch.
 *   2. It is the **baseline** the JIT is benchmarked against, which is what
 *      turns "the JIT is faster" from a claim into a measurement.
 */
int64_t interpret(const Node &program);
