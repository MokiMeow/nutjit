# Milestone 5 — Optimisation

**Goal:** make the generated code genuinely good — and *measure* the
improvement against the naive baseline.

## Concepts

Constant folding on the AST, register allocation instead of the stack machine,
Sethi–Ullman ordering, and peephole rewriting over an instruction list.

## Tasks

- [ ] **Instruction list**: refactor `codegen.cpp` to emit a `vector<Instr>`
      first and encode bytes as a final pass. Peephole optimisation needs
      structure, not raw bytes. Keep the encoder table-driven and tested.
- [ ] **Constant folding** (AST → AST): fold binary nodes with two constant
      children. Do **not** fold division by zero — leave that to run time.
- [ ] **Register allocation**: use a pool of caller-saved registers
      (`RAX, RCX, RDX, RSI, RDI, R8–R11`). Compile the left subtree into
      register *r*, the right into *r+1*, emit `op r, r+1`. Spill to the stack
      only when the tree exceeds the pool; use Sethi–Ullman numbering to compile
      the deeper subtree first and minimise spills.
- [ ] **Peephole**: remove `push rax; pop rax` pairs; shorten
      `movabs rax, imm64` to `mov eax, imm32` (`B8` + 4) when the value fits in
      32 bits (zero-extension is free) — 5 bytes instead of 10.
- [ ] **Benchmark harness**: a `make bench` target timing `fib(30)` (and a loop
      benchmark) for: tree-walking interpreter, naive codegen, optimised
      codegen. Report medians of several runs, plus **JIT compile time**.
- [ ] **Interpreter baseline**: a small tree-walking evaluator over the same AST
      — used only for benchmarking and as a correctness oracle (both paths must
      agree on every test case).

## Files

`src/codegen.cpp` (+ `src/instr.hpp`, `src/regalloc.cpp`, `src/peephole.cpp`),
`src/interp.cpp` (baseline), `src/fold.cpp`, `bench/`, `tests/run-tests.sh`,
`docs/08`.

## Definition of Done

- [ ] Every existing test still passes, and the interpreter and JIT agree on all
      of them (differential testing).
- [ ] Generated code for `2+3*4` is a single `mov` (constant folding works).
- [ ] `push`/`pop` pairs are gone from simple expressions (allocator works).
- [ ] `make bench` prints a table with a measured speedup over the interpreter
      **and** over the naive backend.
- [ ] `make all` warning-free; `make test` green.

## Notes

Correctness first: run the differential test (interpreter vs JIT) after every
optimisation. An optimiser that's fast and wrong is worthless, and register
allocation bugs produce *plausible* wrong numbers rather than crashes.

**Next:** [Milestone 6 — Polish](milestone-6-polish.md).
