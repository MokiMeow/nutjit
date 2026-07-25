# Changelog

All notable changes to nutjit are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project aims
to follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] — 2026-07-25

A small language compiled to native x86-64 at runtime, **887× faster than
interpreting the same AST**.

### Added
- Milestone 6: a REPL that compiles each line to machine code with definitions
  persisting across lines, `--file` and `--interp` modes, a `--bench`
  comparison, and CI gating on a warning-free build plus the test suite.
- Milestone 5: constant folding (AST → AST, with division by zero deliberately
  left to run time), peephole instruction selection (`mov eax, imm32` instead of
  `movabs rax, imm64` where the value fits — 200 → 170 bytes on the benchmark),
  and a tree-walking reference interpreter used as both correctness oracle and
  benchmark baseline.
- Milestone 4: functions with up to six parameters passed in `RDI/RSI/RDX/RCX/
  R8/R9`, `call rel32` with forward-reference patching after all functions are
  emitted, `return`, and recursion — verified to 2000 frames deep.
- Milestone 3: `while` loops with backward jumps, and assignment to existing
  variables.
- Milestone 2: comparison operators (`< > <= >= == !=`) via `cmp`/`setcc`/
  `movzx`, and `if`/`else` using jump **backpatching**.
- Milestone 1: `let` bindings, a compile-time environment mapping names to
  frame slots, and real stack frames (`push rbp`/`mov rbp,rsp`/`sub rsp,N`
  with N rounded to 16 so `RSP` stays aligned at every call).
- 51 tests, every one a **differential test** requiring the JIT and the
  interpreter to agree.

### Verified
- `fib(30)` = 832040 from JIT-compiled machine code; all three back ends agree.
- Interpreter 5075.75 ms vs JIT 5.72 ms — **887.1×**, with 0.008 ms compile time.
- `2 + 3 * 4 - 1` folds to a single `mov eax, 13`.

### Known limitations
- The code generator is still a **stack machine**: every binary operation
  round-trips through `push`/`pop`. Register allocation was specified in
  milestone 5 and deliberately **not** implemented; it is the first stretch
  goal and is documented as deferred rather than claimed.
- No strings, heap, or types beyond 64-bit integers.
- x86-64 / System V only.

## [0.1.0] — milestone 0
- First working version: compiles integer arithmetic to x86-64 machine code at
  runtime and executes it.
