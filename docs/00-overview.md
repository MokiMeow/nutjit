# 00: Overview

## What nutjit is

nutjit is a small programming language whose implementation is a **just-in-time
compiler**: it translates source code into x86-64 machine code at runtime,
writes those bytes into executable memory, and calls them. There is no
interpreter in the execution path and no LLVM behind the curtain: the code
generator is a few hundred lines of C++ that emits opcode bytes.

## The one-sentence idea

> Turn text into machine code in memory, then jump to it.

## Design goals

1. **Every stage is ours and readable.** Lexer, parser, code generator, and
   executable-memory management are all hand-written and commented with the
   *why* (including instruction encodings).
2. **Always working, always tested.** Each milestone leaves a compiler that
   builds clean and passes a test suite whose expected values are produced by
   JIT-compiled code.
3. **Simple structures, real results.** A stack-machine baseline first, then
   bounded scratch-register allocation, folding, and compact immediates, so
   each improvement remains readable and measurable.
4. **Zero cost, zero dependencies.** g++ and POSIX. Nothing else.

## What it is *not*

- Not a general-purpose language: no strings, objects, or GC in v1.
- Not portable: it emits x86-64 (see [ADR 0003](decisions/0003-x86-only.md)).
- Not an optimising compiler in the LLVM sense; milestone 5 is deliberately
  modest (register allocation, constant folding, peephole).

## Pipeline

```
source text
    │  lexer          src/lexer.cpp      -> tokens
    ▼
  tokens
    │  parser + validator               -> AST (include/ast.hpp)
    ▼
   AST
    │  codegen        src/codegen.cpp    -> std::vector<uint8_t>
    ▼
machine code bytes
    │  JitBuffer      src/jitmem.cpp     -> mmap RW, copy, mprotect RX
    ▼
callable function ── run() ──▶ int64_t
```

Milestone 0 implemented this spine for integer arithmetic. Later milestones
widened the language while preserving the same boundaries; the interpreter is
an independent backend over the validated AST.

Read the [architecture doc](02-architecture.md) next, or
[getting started](01-getting-started.md) to run it.
