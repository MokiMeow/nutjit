# 04 — Roadmap

From "JITs arithmetic" (today) to "compiles a small language with recursive
functions to native code." Each milestone leaves a working, tested compiler.

## The plan

| # | Milestone | You'll build | You'll learn |
|---|-----------|--------------|--------------|
| 0 | **Arithmetic JIT** ✅ | lexer, parser, x86-64 codegen, executable memory | the whole pipeline, instruction encoding, W^X |
| 1 | **Variables** | `let x = …;`, an environment, stack frames | prologue/epilogue, locals at `[rbp-N]` |
| 2 | **Conditionals** | `< > == !=`, `if`/`else` | `cmp`/`setcc`, `jcc`, backpatching jump targets |
| 3 | **Loops** | `while` | backward jumps, block structure in codegen |
| 4 | **Functions** | definitions, calls, arguments, recursion | System V arg registers, `call`/`ret`, stack alignment |
| 5 | **Optimisation** | register allocation, constant folding, peephole | why the stack machine was slow; real codegen quality |
| 6 | **Polish** | REPL, benchmark vs interpreter, CI, `v1.0.0` | measurement and presentation |

## Dependency order

```
M0 ─► M1 ─► M2 ─► M3 ─► M4 ─► M5 ─► M6
```

Strictly linear: variables need stack frames before conditionals need blocks;
loops reuse M2's jump machinery; functions need all of it; optimisation needs
something worth optimising.

## Definition of Done (whole project)

nutjit compiles a program with variables, `if`/`else`, `while`, and recursive
functions to x86-64 machine code at runtime and executes it correctly —
demonstrated by `fib(30)` returning 832040 — with a published benchmark against
a tree-walking interpreter of the same AST, a REPL, green CI, and a `v1.0.0`
tag.

## The headline artifact

A README table like:

| implementation | `fib(30)` |
|---|---|
| tree-walking interpreter | ~X ms |
| nutjit (JIT to native) | ~Y ms |
| **speedup** | **Nx** |

That single table is the payoff of the whole project: it proves the machine code
is real and that you understand *why* it's faster.

## Stretch goals (after v1.0.0)

- Strings and a tiny heap.
- A second backend (ARM64) behind the same AST — proves the design separates
  front end from code generation.
- SSA-based IR and a smarter register allocator (linear scan).
- Inline caching or a simple tracing tier.
- `objdump`-verified encoding tests (assemble expectations, compare bytes).
