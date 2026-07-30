# ADR 0001: Emit machine code ourselves (not LLVM, not an assembler)

**Status:** accepted · **Date:** 2026

## Context

A language implementation needs a backend. Options:

1. **Tree-walking interpreter**: no code generation at all.
2. **Emit assembly text**, then shell out to `as`/`gcc` and `dlopen` the result.
3. **Use LLVM** (or libgccjit, libjit) as the backend.
4. **Emit machine-code bytes directly** into executable memory.

## Decision

**Emit x86-64 machine-code bytes ourselves** and execute them from `mmap`'d
pages.

## Rationale

- Options 1–3 all skip the thing this project exists to demonstrate. An
  interpreter never touches machine code; an assembler pipeline outsources
  encoding and adds a process spawn per compile (not a JIT); LLVM is a
  ~30-million-line dependency where the interesting work: instruction
  selection, register allocation, encoding: is done *for* you.
- Emitting bytes is what makes this a **JIT**: compile in memory, jump in,
  no external tools, microsecond compile times.
- The knowledge is transferable and rare: REX prefixes, ModR/M, W^X memory, and
  calling conventions are the substrate under every runtime (JVM, V8, LuaJIT).

## Consequences

- Bugs surface as SIGSEGV/SIGILL rather than compiler errors, so
  `objdump`-based verification and a strong test suite are mandatory (see
  [docs/09](../09-testing-and-debugging.md)).
- The backend is architecture-specific ([ADR 0003](0003-x86-only.md)).
- We must obey the platform ABI ourselves ([docs/07](../07-calling-convention.md)).
- Milestone 6 adds a tree-walking interpreter **only** as a benchmark baseline,
  never as the execution path.
