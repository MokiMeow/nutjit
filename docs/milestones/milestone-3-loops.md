# Milestone 3: Loops ✅ (done)

**Goal:** `while` loops: the first *backward* jumps, plus assignment so a loop
can make progress.

## Concepts

Backward jump displacements (known immediately, no patching needed), loop
structure in a single-pass code generator, and mutation of locals.

## Tasks

- [x] **Parser**: `while (cond) { body }`; and assignment to an existing
      variable (`x = expr;`) as a statement, distinct from `let`.
- [x] **Codegen**:
  - record `loop_start` = current code offset;
  - compile the condition; `test rax, rax`; `je rel32` **forward** placeholder
    to the loop exit;
  - compile the body;
  - `jmp rel32` **backward** to `loop_start`: the displacement is
    `loop_start - (position_after_this_jmp)`, computable right away;
  - patch the exit `je`.
- [x] Assignment reuses milestone 1's slot lookup and `mov [rbp-off], rax`.
- [x] **Tests**: a counting loop
      (`let i = 0; let s = 0; while (i < 5) { s = s + i; i = i + 1; } s;` → 10);
      a loop that never executes; a nested loop.

## Files

`src/parser.cpp`, `include/ast.hpp`, `src/codegen.cpp`, `tests/run-tests.sh`,
`docs/05`.

## Definition of Done

- [x] The counting loop returns the correct sum from generated code.
- [x] A zero-iteration loop (`while (0 < 0)`) is skipped correctly.
- [x] A nested loop produces the right result.
- [x] `make all` warning-free; `make test` green.

## Notes

The classic bug is computing the backward displacement from the *start* of the
`jmp` rather than the byte *after* it: a 5-byte error that usually lands
mid-instruction and raises SIGILL. Reuse the same helper the forward patcher
uses so the convention lives in exactly one place.

**Next:** [Milestone 4: Functions](milestone-4-functions.md).
