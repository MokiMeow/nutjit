# Milestone 1 — Variables

**Goal:** `let` bindings and variable references, stored in a real stack frame.

## Concepts

Identifiers and keywords, statements vs expressions, a compile-time environment
mapping names to slots, and the function prologue/epilogue that makes
`[rbp-N]` addressing work.

## Tasks

- [ ] **Lexer**: scan identifiers (`[A-Za-z_][A-Za-z0-9_]*`); a keyword table
      recognising `let`; add `Ident`, `Let`, `Assign` (`=`), `Semicolon`.
- [ ] **AST**: add `Let` (name + initialiser), `Var` (name), and a `Block`/
      program node holding a statement list.
- [ ] **Parser**: `program := statement*`,
      `statement := 'let' IDENT '=' expr ';' | expr ';'`. The value of the
      program is its last expression statement.
- [ ] **Environment**: a compile-time map name → slot index, assigned in
      declaration order; report a use of an undeclared name as an error.
- [ ] **Codegen**:
  - prologue `push rbp` (`55`), `mov rbp, rsp` (`48 89 E5`),
    `sub rsp, N` (`48 81 EC imm32`), with **N rounded up to a multiple of 16**;
  - store `mov [rbp-off], rax` (`48 89 45 disp8` for small offsets);
  - load `mov rax, [rbp-off]` (`48 8B 45 disp8`);
  - epilogue `leave` (`C9`) + `ret` (`C3`).
- [ ] **Tests**: `let x = 5; x * 2;` → 10; shadowing/redeclaration behaviour;
      multiple variables; using an undeclared name must be rejected.

## Files

`src/lexer.cpp`, `include/lexer.hpp`, `include/ast.hpp`, `src/parser.cpp`,
`src/codegen.cpp`, `tests/run-tests.sh`, and `docs/03`/`docs/05` updates.

## Definition of Done

- [ ] `let a = 3; let b = 4; a * b + 1;` returns 13 from generated code.
- [ ] Undeclared variables are rejected with a clear message and offset.
- [ ] Frame size is a multiple of 16 (alignment holds for milestone 4).
- [ ] `make all` warning-free; `make test` green including the new cases.

## Notes

Use `disp8` (`48 89 45 F8`) while offsets fit in a signed byte, and fall back to
`disp32` (`48 89 85 imm32`) beyond that — or just always use `disp32` for
simplicity and note the trade-off.

## References

- [docs/07 — Calling convention](../07-calling-convention.md) (frames, alignment)

**Next:** [Milestone 2 — Conditionals](milestone-2-conditionals.md).
