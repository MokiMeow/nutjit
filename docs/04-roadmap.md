# 04: Roadmap

All planned milestones are complete. The v1.1 audit closed the correctness and
polish gaps found after the original v1.0 release.

| # | Milestone | Result |
|---|-----------|--------|
| 0 | Arithmetic JIT | lexer, parser, x86-64 bytes, W^X execution |
| 1 | Variables | `let`, assignment, aligned local frames |
| 2 | Conditionals | comparisons, `if`/`else`, jump backpatching |
| 3 | Loops | `while` and backward jumps |
| 4 | Functions | arguments, calls, returns, recursion |
| 5 | Optimisation | folding, immediates, scratch registers, interpreter oracle |
| 6 | Polish | persistent REPL, median benchmark, tests, demo, CI, releases |

## Representative verified result

| implementation | fib(30) run | compile | speedup |
|---|---:|---:|---:|
| tree-walking interpreter | 3507.69 ms |: | 1.0× |
| naive JIT | 5.08 ms | 0.001 ms | 690.9× |
| optimized JIT | 4.72 ms | 0.002 ms | 742.5× |

Code size was 208 bytes naive and 151 bytes optimized in that run. Timing
varies by host; the benchmark uses repeated medians and verifies all three
backends return `fib(30) = 832040`.

## Definition of Done: met

nutjit compiles variables, control flow, and recursive functions to native
x86-64 at runtime; rejects invalid programs before execution; agrees with an
independent interpreter across the full suite; provides a persistent REPL; and
is gated by behavioral, byte-exact, build, sample, and benchmark CI checks.

## Future work

- A real IR with liveness analysis and a larger allocator.
- An ARM64 backend with instruction-cache synchronization.
- Strings, heap allocation, and richer types.
- Profile-guided or tracing optimization.
