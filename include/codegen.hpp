#pragma once
#include <cstdint>
#include <vector>

#include "ast.hpp"

/* Stage 3: AST -> x86-64 machine code.
 *
 * Emits every function into one buffer (so `call rel32` displacements are
 * valid between them) and returns it along with the byte offset of the entry
 * point — the implicit `main` built from the program's top-level statements.
 *
 * The generated entry point is a System V function taking no arguments and
 * returning its value in RAX, so the host can call it as `int64_t(*)()`. */

struct CompiledProgram {
    std::vector<uint8_t> code;
    size_t entry_offset = 0;
};

/* `optimise` enables constant folding and the peephole pass (milestone 5).
 * Both back ends must agree with the interpreter either way — that equality is
 * what the differential tests check. */
CompiledProgram codegen(const Node &program, bool optimise = true);
