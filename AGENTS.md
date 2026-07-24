# AGENTS.md — operating manual for coding agents

Source of truth for anyone (human or AI) working on nutjit. Read it fully before
making changes. If anything here conflicts with a stray note elsewhere, **this
file wins.**

---

## 1. Roles

- **Orchestrator** — plans milestones, defines Definitions of Done, reviews
  diffs, keeps docs honest.
- **Builder** — implements one milestone at a time against `docs/milestones/`,
  keeping the build green and the tests passing.

Builder loop: **pick the lowest-numbered unfinished milestone → implement it →
build clean → make the tests prove it → tick its Definition of Done → update
docs/CHANGELOG → commit → next.**

## 2. Ground rules (non-negotiable)

1. **The build must never break and the tests must always pass.** Every commit:
   `make clean && make all` with zero warnings, and `make test` green.
2. **Tests are the proof.** Every new language feature adds cases to
   `tests/run-tests.sh`. A milestone is not done until its feature is covered by
   passing tests whose values were computed by **JIT-compiled code**.
3. **No LLVM, no libjit, no assembler.** We emit machine-code bytes ourselves.
   That is the entire point of the project ([ADR 0001](docs/decisions/0001-own-codegen.md)).
4. **No dependencies** beyond the C++17 standard library and POSIX
   (`mmap`/`mprotect`).
5. **New `.cpp` → `src/`, new `.hpp` → `include/`.** The Makefile globs
   `src/*.cpp`; no Makefile edits needed for new files.
6. **Verify encodings against the manual.** When you add an instruction, put its
   byte encoding in a comment (opcode, REX, ModR/M) as the existing code does.
   A wrong encoding usually crashes with SIGSEGV/SIGILL, not a nice error.
7. **No feature without a doc.** Update the relevant `docs/` page in the same
   milestone.

## 3. Build, run, verify

| Command | What it does |
|---------|--------------|
| `make all` | Build `build/nutjit` (zero warnings expected). |
| `make run` | Compile a sample expression, dump its machine code, run it. |
| `make test` | Run `tests/run-tests.sh` — the expression suite. |
| `make clean` | Remove `build/`. |

**Definition of "it works":**
1. `make clean && make all` — no warnings.
2. `make test` — all cases pass, exit 0.
3. The new feature's own test cases return correct values.

Never claim a milestone is done without running the tests and reading the
output.

## 4. Coding standards

- **C++17**, 4-space indent, `snake_case` for functions/variables,
  `PascalCase` for types, trailing `_` for private members.
- Prefer plain functions and small classes; no template metaprogramming, no
  inheritance hierarchies. Readability is the product.
- Throw `std::runtime_error` with a message including the source offset for user
  errors; `main` catches and prints them. Never `exit()` deep in the code.
- **Machine-code emission**: one helper per instruction where it aids clarity,
  each with a comment giving the encoding. Keep the emitter's byte-level code
  obvious rather than clever.
- Comment *why* (which encoding, which convention, which invariant), not *what*.

## 5. Commit and branch style

- `type(scope): outcome`, imperative, lower case.
  Examples: `feat(codegen): emit conditional jumps with backpatching`,
  `fix(parser): reject a trailing operator`.
- Types: `feat`, `fix`, `docs`, `refactor`, `build`, `chore`, `test`.
- **No AI/co-author trailers.**
- Branch per milestone (`milestone-2-conditionals`), PR into `main`, CI green.

## 6. The milestone path

Specs with Definitions of Done live in `docs/milestones/`.

| # | Milestone | Adds | Spec |
|---|-----------|------|------|
| 0 | Arithmetic JIT | lexer, parser, x86-64 codegen, executable memory | [spec](docs/milestones/milestone-0-arithmetic-jit.md) ✅ |
| 1 | Variables | `let`, environment, stack frames, locals at `[rbp-N]` | [spec](docs/milestones/milestone-1-variables.md) |
| 2 | Conditionals | comparisons, `if`/`else`, `cmp`/`setcc`, jump backpatching | [spec](docs/milestones/milestone-2-conditionals.md) |
| 3 | Loops | `while`, backward jumps, loop-scoped codegen | [spec](docs/milestones/milestone-3-loops.md) |
| 4 | Functions | multiple functions, args in registers, recursion, `fib` | [spec](docs/milestones/milestone-4-functions.md) |
| 5 | Optimisation | register allocation, constant folding, peephole | [spec](docs/milestones/milestone-5-optimisation.md) |
| 6 | Polish | REPL, JIT-vs-interpreter benchmark, CI, tag `v1.0.0` | [spec](docs/milestones/milestone-6-polish.md) |

**Definition of Done (whole project):** nutjit compiles a small language with
variables, control flow, and recursive functions to native x86-64 at runtime,
runs `fib(30)` correctly, and publishes a measured speedup over a tree-walking
interpreter of the same AST.

## 7. What NOT to do

- Do not add an interpreter as the *primary* execution path (milestone 6 adds
  one only as a benchmark baseline).
- Do not shell out to an assembler or linker.
- Do not skip `make test` because "the codegen looks right" — a wrong byte is a
  crash, not a warning.
- Do not break existing tests to make a new feature fit; fix the design.

## 8. Tools reference

- **g++** (C++17) — the only compiler needed.
- **`mmap`/`mprotect`** (POSIX) — executable memory; see `src/jitmem.cpp`.
- **Intel SDM / [x86 reference](https://www.felixcloutier.com/x86/)** — the
  authority for every byte you emit.
- **`objdump -D -b binary -m i386:x86-64`** — disassemble a `--dump` to check
  your encodings (see [docs/09](docs/09-testing-and-debugging.md)).

Build one milestone, test it, document it, commit it. Then the next.
