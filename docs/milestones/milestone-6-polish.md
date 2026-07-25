# Milestone 6 — Polish ✅ (done)

**Goal:** a REPL, an honest benchmark, green CI, and `v1.0.0`.

## What shipped

### Proof it works
- [x] `tests/run-tests.sh` covers every language feature — arithmetic,
      precedence, associativity, variables, assignment, all six comparisons,
      `if`/`else` including nesting, `while` including zero-iteration and
      nested loops, functions, forward references, recursion, and rejected
      syntax/name errors. **51 cases.**
- [x] Every case is a **differential test**: the program runs through the JIT
      *and* the reference interpreter, and both must agree. This is what catches
      codegen that returns a plausible wrong number.
- [x] CI on an x86-64 runner: warning-free build, `make test`, and `make bench`
      as a smoke run.

### Presentation
- [x] The benchmark table at the top of the README, with the conditions stated
      and the flat optimised row explained rather than hidden.
- [x] A `--dump` walkthrough showing real machine code, including the folded
      `mov eax, 13`.
- [x] The pipeline diagram (source → lexer → parser → AST → codegen → bytes →
      `mmap` → CALL) linking to [docs/05](../05-x86-codegen.md).

### Features
- [x] A **REPL** (`--repl`): compiles each line to machine code and runs it,
      with function definitions persisting across lines.
- [x] `--file` to run a program from disk; `--interp` to run through the
      reference back end; `--bench` for the comparison.

### Hygiene
- [x] All status tables accurate; every milestone's Definition of Done ticked
      (and milestone 5's deferred item marked deferred, not ticked).
- [x] `CHANGELOG.md` updated.
- [x] Tagged `v1.0.0`.

## Not done

- [ ] ~~Golden-hex encoding tests~~ — the differential tests catch wrong
      *behaviour*; byte-exact assertions would additionally catch a wrong
      *encoding* that happens to behave correctly. Worth adding, not done.
- [ ] ~~An asciinema/GIF of the REPL~~ — the README shows transcripts instead.

Listed rather than silently dropped.

## Definition of Done

- [x] CI green on `main`: warning-free build, all tests, bench runs.
- [x] README opens with the benchmark table and a working demo.
- [x] The REPL works for expressions, variables, and function definitions.
- [x] `v1.0.0` tagged.

## Verified

```
51/51 tests pass (JIT and interpreter agree on every one)
fib(30) = 832040 from JIT-compiled machine code
887× faster than the tree-walking interpreter
2000-deep recursion holds — stack alignment is correct
```

## Stretch goals (after v1.0.0)

- **Register allocation** — the deferred half of milestone 5, and the biggest
  remaining performance win.
- An **ARM64 backend** behind the same AST (remember the instruction-cache
  flush that x86-64 does not need).
- Strings and a small heap; an SSA IR with linear-scan allocation; a tracing
  tier that re-optimises hot loops.
