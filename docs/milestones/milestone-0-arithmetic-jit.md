# Milestone 0: Arithmetic JIT ✅ (done)

**Goal:** compile an integer arithmetic expression to x86-64 machine code at
runtime and execute it.

## Concepts

The full compiler spine: hand-written lexing, recursive-descent parsing with
precedence, x86-64 instruction encoding (REX/ModR/M/immediates), and W^X
executable memory.

## What shipped

- [x] `src/lexer.cpp`: numbers, `+ - * / ( )`, source offsets, errors.
- [x] `src/parser.cpp`: recursive descent; precedence and left associativity
      from the grammar; unary minus desugared to `0 - x`.
- [x] `include/ast.hpp`: `Number` / `Binary` nodes with `unique_ptr` children.
- [x] `src/codegen.cpp`: stack-machine codegen emitting `movabs`, `push`,
      `pop`, `mov`, `add`, `sub`, `imul`, `cqo`+`idiv`, `ret`.
- [x] `src/jitmem.cpp`: `JitBuffer`: `mmap` RW → copy → `mprotect` RX → call;
      RAII cleanup, non-copyable.
- [x] `src/main.cpp`: CLI with `--dump`, stdin fallback, error reporting.
- [x] `tests/run-tests.sh`: 16 cases (arithmetic, precedence, associativity,
      unary minus, 64-bit result, rejected syntax errors).

## Definition of Done

- [x] `make clean && make all` builds with **zero warnings**.
- [x] `make test` passes all 16 cases (values computed by generated code).
- [x] `make run` prints the machine code and the correct result
      (`2 + 3 * (10 - 4) / 2` → 86 bytes → `11`).

## Verified output

```
nutjit: 86 bytes of machine code
48 b8 02 00 00 00 00 00 00 00 50 48 b8 03 00 00 ...
11
```

## References

- [felixcloutier.com/x86](https://www.felixcloutier.com/x86/)
- [docs/05: x86-64 codegen](../05-x86-codegen.md),
  [docs/06: JIT memory](../06-jit-memory.md)

**Next:** [Milestone 1: Variables](milestone-1-variables.md).
