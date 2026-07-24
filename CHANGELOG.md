# Changelog

All notable changes to nutjit are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project aims
to follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Milestone 0: the complete JIT spine — hand-written lexer, recursive-descent
  parser with precedence and left associativity, an x86-64 code generator
  emitting real instruction bytes (`movabs`, `push`/`pop`, `add`, `sub`,
  `imul`, `cqo`+`idiv`, `ret`), and W^X executable memory (`mmap` RW → copy →
  `mprotect` RX) that the host calls as a function pointer.
- CLI with `--dump` to print the generated machine code, and a stdin fallback.
- `tests/run-tests.sh`: 16 cases covering arithmetic, precedence, associativity,
  unary minus, 64-bit results, and rejected syntax errors — all values produced
  by executing generated code.
- Build system (`make all` / `run` / `test` / `clean`) and CI.
- Documentation set under `docs/` (architecture, codegen, JIT memory, calling
  convention, optimisation, debugging, glossary), 3 ADRs, 7 milestone specs, and
  the `AGENTS.md` operating manual.

## [0.1.0] — milestone 0
- First working version: compiles integer arithmetic to x86-64 machine code at
  runtime and executes it.
