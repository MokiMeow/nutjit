# Milestone 6: Polish ✅

**Goal:** a persistent REPL, an honest benchmark, comprehensive tests, green
CI, and reproducible releases.

## What shipped

- [x] 63 behavioral cases covering every language feature and important
      rejection path.
- [x] Every valid case executes through the JIT and interpreter and must agree;
      every invalid case must be rejected by both modes.
- [x] Six byte-level checks covering folded constants, scratch-register
      arithmetic, signed immediates, and nested-call alignment.
- [x] A REPL whose successful variables and functions persist across inputs.
- [x] `--file`, `--interp`, `--dump`, and a repeated-median `--bench`.
- [x] A README benchmark, architecture links, and recorded REPL demonstration.
- [x] CI gates a `-Werror` build, all tests, a sample run, and the benchmark
      smoke test.
- [x] Accurate status docs and changelog.
- [x] `v1.0.0` for the original completed project and `v1.1.0` for the audited,
      hardened release.

## Definition of Done

- [x] CI is green on `main`.
- [x] README opens with a working demo and measured benchmark.
- [x] The REPL works for expressions, variables, and functions across inputs.
- [x] Behavioral and byte-exact tests are green.
- [x] A release tag identifies the verified main commit.

## Verified

```
63/63 behavioral tests pass in both backends
6/6 encoding properties pass
fib(30) = 832040 from interpreter, naive JIT, and optimized JIT
2000-deep recursion holds with aligned nested calls
```

## Future extensions

- A larger allocator backed by an IR and liveness analysis.
- An ARM64 backend with explicit instruction-cache synchronization.
- Strings, a small heap, and richer types.
- A tracing tier for hot loops.
