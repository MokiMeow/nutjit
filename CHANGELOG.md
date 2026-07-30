# Changelog

All notable changes to nutjit are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Expanded the README with the complete execution architecture, supported
  scope, limitations, and documentation map.
- Standardized first-party Markdown punctuation.

## [1.1.0]: 2026-07-25

### Added

- A semantic validation pass for duplicate declarations, call arity, unknown
  names, top-level returns, and variables not declared on every control-flow
  path.
- A caller-saved scratch-register allocator and signed-immediate instruction
  selection in the optimized backend.
- Byte-exact golden tests for constant folding, scratch-register arithmetic,
  immediates, and nested-call stack alignment.
- Differential coverage for invalid programs, signed wraparound, declarations
  across control flow, and persistent REPL state, bringing the suite to 63
  behavioral cases.
- A recorded REPL demonstration in SVG and asciinema cast formats.

### Fixed

- Keep successful variables and function definitions alive across REPL inputs.
- Align `RSP` before generated calls even when expression temporaries are
  currently spilled.
- Make interpreter and constant-folder signed arithmetic match x86-64
  two's-complement wrapping without invoking C++ signed-overflow behavior.
- Reject integer literals larger than `INT64_MAX` instead of overflowing while
  scanning.
- Reject wrong-arity calls and duplicate parameters/functions before either
  backend runs.

### Changed

- Benchmark interpreter, compilation, and JIT execution with repeated median
  samples.
- CI now enforces `-Werror`, runs the real benchmark smoke test, and uses the
  current Node 24 checkout action.

## [1.0.0]: 2026-07-25

### Added

- Arithmetic, variables, assignment, comparisons, conditionals, loops,
  functions with up to six parameters, forward calls, returns, and recursion.
- A hand-written x86-64 backend with real stack frames and System V argument
  passing.
- W^X executable memory, constant folding, compact literal loads, a
  tree-walking interpreter, a REPL, benchmark harness, and CI.
- 51 differential behavioral tests.

### Verified

- `fib(30)` returned 832040 from generated machine code.
- `2 + 3 * 4 - 1` folded to one `mov eax, 13`.

### Known limitations

- The original release still used stack spills for every binary expression;
  v1.1.0 replaces the common path with scratch registers.
- No strings, heap, or types beyond signed 64-bit integers.
- System V x86-64 only.

## [0.1.0]: milestone 0

- First working version: compile integer arithmetic to x86-64 machine code at
  runtime and execute it.
