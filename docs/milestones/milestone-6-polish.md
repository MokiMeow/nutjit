# Milestone 6 — Polish (portfolio pass)

**Goal:** turn a working JIT into a repo that impresses on sight, and tag
`v1.0.0`.

## Tasks

### Proof it works
- [ ] Expand `tests/run-tests.sh` to cover every language feature, plus the
      differential check (interpreter vs JIT agree) from milestone 5.
- [ ] Add encoding tests: for a few known programs, assert the emitted bytes
      match a recorded golden hex string, so an encoding regression fails loudly
      instead of crashing mysteriously.
- [ ] CI on an x86-64 runner: build with zero warnings, run `make test`, run
      `make bench` (smoke, not thresholds).

### Presentation
- [ ] **The benchmark table in the README** — the headline artifact:

      | implementation | fib(30) | vs interpreter |
      |---|---|---|
      | tree-walking interpreter | X ms | 1.0× |
      | nutjit (naive codegen) | Y ms | N× |
      | nutjit (optimised) | Z ms | M× |

      State the CPU, the number of runs, and that the figure is a median.
- [ ] An asciinema/GIF of the REPL compiling and running code, plus a `--dump`
      showing real machine code, at the top of the README.
- [ ] A short "how a line of source becomes machine code" walkthrough in the
      README linking to [docs/05](../05-x86-codegen.md).

### Features
- [ ] A **REPL**: read a line, compile it, run it, print the result, keep
      definitions across lines. This is what makes the project feel alive.

### Hygiene
- [ ] All docs' status tables accurate; every milestone DoD ticked.
- [ ] `CHANGELOG.md` updated; items moved from Unreleased to `1.0.0`.
- [ ] Tag the release: `git tag v1.0.0`.

## Definition of Done

- [ ] CI green on `main`: warning-free build, all tests, bench runs.
- [ ] README opens with the demo and the benchmark table.
- [ ] The REPL works for expressions, variables, and function definitions.
- [ ] `v1.0.0` tagged.

## Stretch goals (after v1.0.0)

- An **ARM64 backend** behind the same AST — the strongest proof the design
  separates front end from codegen. Remember the instruction-cache flush.
- Strings and a small heap; an SSA IR with linear-scan allocation; a tracing
  tier that re-optimises hot loops.
