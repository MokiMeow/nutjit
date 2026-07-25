# Milestone 2 — Conditionals ✅ (done)

**Goal:** comparison operators and `if`/`else`, which means jumps — and
therefore **backpatching**.

## Concepts

`cmp` + `setcc` to turn a comparison into 0/1, conditional jumps (`jcc`), and
patching a jump's displacement after the target position is known.

## Tasks

- [x] **Lexer/parser**: `<`, `>`, `<=`, `>=`, `==`, `!=` at a precedence level
      below `+`/`-`; `if (cond) { … } else { … }` with a `Block` node.
- [x] **Comparison codegen**: with left in `RAX`, right in `RCX`:
      `cmp rax, rcx` (`48 39 C8`), then `setcc al`
      (`0F 9C C0` = `setl`, `9F` `setg`, `9E` `setle`, `9D` `setge`,
      `94` `sete`, `95` `setne`), then `movzx rax, al` (`48 0F B6 C0`).
- [x] **Branch codegen** with backpatching:
  - compile the condition → `RAX`; `test rax, rax` (`48 85 C0`);
  - `je rel32` (`0F 84` + 4 bytes) with a **placeholder** displacement, and
    record the byte position of that displacement;
  - compile the then-branch; `jmp rel32` (`E9` + 4) placeholder over the else;
  - patch the `je` displacement to the else's start;
  - compile the else-branch; patch the `jmp` to the end.
- [x] Displacements are **relative to the end of the jump instruction**; a
      helper (`patch_rel32(at, target)`) keeps this in one place.
- [x] **Tests**: `if (1 < 2) { 10; } else { 20; }` → 10 (and the inverse → 20);
      each comparison operator; a nested `if`; comparison as a value
      (`3 < 5;` → 1).

## Files

`src/lexer.cpp`, `src/parser.cpp`, `include/ast.hpp`, `src/codegen.cpp`
(emitter gains label/patch helpers), `tests/run-tests.sh`, `docs/05`.

## Definition of Done

- [x] Both branches of an `if`/`else` return the right value, verified by tests.
- [x] All six comparison operators tested.
- [x] Nested conditionals work — proof the patching is position-independent.
- [x] `make all` warning-free; `make test` green.

## Notes

Backpatching is where off-by-one errors live. If a jump lands one byte off you
usually get SIGILL — disassemble (`objdump`) and check the target lands exactly
on an instruction boundary.

**Next:** [Milestone 3 — Loops](milestone-3-loops.md).
