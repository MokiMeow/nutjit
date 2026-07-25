# Milestone 5 — Optimisation ✅

**Goal:** improve generated code and measure it against a stable naive
baseline.

## What shipped

- [x] Constant folding over the AST. Division by zero and signed division
      overflow remain runtime operations.
- [x] Defined two's-complement wrapping for folded addition, subtraction, and
      multiplication.
- [x] Compact literal loads and signed-immediate forms for arithmetic and
      comparisons.
- [x] A small caller-saved `R10`/`R11` scratch-register pool. The backend
      spills only when it exhausts the pool or must preserve a value across a
      nested call.
- [x] A tree-walking reference interpreter used as the differential-test oracle
      and benchmark baseline.
- [x] A median-based benchmark for interpreter, naive JIT, and optimized JIT.
- [x] Byte-exact tests for representative optimized encodings.

## Verified result

One representative WSL2 run:

```
implementation           run (ms) compile (ms)    speedup
tree-walking interp       3507.69            -       1.0x
nutjit (naive)               5.08        0.001     690.9x
nutjit (optimised)           4.72        0.002     742.5x

code size: naive 208 bytes, optimised 151 bytes
```

The exact timing depends on the host. The durable claims are that all backends
return 832040 for `fib(30)`, the harness uses repeated medians, and the
optimized encoding is smaller for this workload.

## Definition of Done

- [x] Every behavioral test passes through both the interpreter and JIT.
- [x] `2 + 3 * 4 - 1` compiles to a single `mov`.
- [x] Common expression intermediates stay in scratch registers.
- [x] Spills preserve correctness for deep expressions and nested calls.
- [x] `make bench` reports median run and compile times.
- [x] Optimized code is measurably smaller than naive code.
- [x] `make all` is warning-free and `make test` is green.

**Next:** [Milestone 6 — Polish](milestone-6-polish.md).
