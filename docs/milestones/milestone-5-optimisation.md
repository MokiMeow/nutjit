# Milestone 5 — Optimisation ✅ (done, with one part deliberately deferred)

**Goal:** improve the generated code — and *measure* the result against the
naive baseline rather than asserting it.

## Concepts

Constant folding on the AST, peephole instruction selection, and using a second
back end as both a correctness oracle and a benchmark baseline.

## What shipped

- [x] **Constant folding** (AST → AST): binary nodes with two constant children
      collapse to a single `Number`. Division by zero is **not** folded — that
      would turn a runtime fault into a compile-time surprise.
- [x] **Peephole instruction selection**: a value that fits in 32 bits emits
      `mov eax, imm32` (`B8` + 4 bytes) instead of `movabs rax, imm64`
      (`48 B8` + 8). Writing `eax` zero-extends into `rax`, so it is free —
      5 bytes instead of 10.
- [x] **Reference interpreter** (`src/interp.cpp`): a tree-walking evaluator
      over the same AST. It is the **oracle** for differential testing and the
      **baseline** for the benchmark.
- [x] **Benchmark harness** (`make bench`): times `fib(30)` for the
      interpreter, naive codegen, and optimised codegen, and reports JIT compile
      time alongside run time.
- [x] **Differential testing**: all 51 test cases run through both back ends and
      must agree.

## Deliberately not done — register allocation

The spec originally called for replacing the stack machine with a register
allocator (a pool of caller-saved registers, Sethi–Ullman ordering, spilling
only when the expression tree exceeds the pool).

**This was not implemented.** The code generator is still a stack machine:
every binary operation round-trips its left operand through `push`/`pop`. It is
listed as the first stretch goal in [the roadmap](../04-roadmap.md), and the
README says so plainly rather than implying the codegen is better than it is.

Saying this out loud costs nothing and is the difference between a milestone
that is honest and one that quietly overclaims.

## Verified result

```
implementation           run (ms) compile (ms)    speedup
tree-walking interp       5075.75            -       1.0x
nutjit (naive)               5.72        0.008     887.1x
nutjit (optimised)           5.78        0.033     877.5x

code size: naive 200 bytes, optimised 170 bytes
```

Constant folding on a folded expression:

```
$ nutjit --dump "2 + 3 * 4 - 1;"
b8 0d 00 00 00        # mov eax, 13 — the whole expression, one instruction
```

## Definition of Done

- [x] Every test passes, and the interpreter and JIT agree on all of them.
- [x] `2 + 3 * 4 - 1` compiles to a single `mov` (folding works).
- [x] `make bench` prints a measured table including compile time.
- [x] Optimised code is measurably **smaller** (200 → 170 bytes).
- [x] `make all` warning-free; `make test` green.
- [ ] ~~`push`/`pop` pairs removed by a register allocator~~ — **deferred**, see
      above.

## Notes

The optimised row is *not* faster than naive on `fib(30)`, because `fib` has no
constant subexpressions and the peephole change affects size, not hot-loop
speed. That flat result is reported as-is. The enormous win in this milestone
is the JIT itself — 887× over interpretation for 8 microseconds of compile
time — not the optimiser.

**Next:** [Milestone 6 — Polish](milestone-6-polish.md).
