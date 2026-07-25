# 02 — Architecture

Data flows through explicit stages:

```
source
  → lexer (tokens with byte offsets)
  → parser + semantic validator (AST)
  → optional AST constant folding
  → x86-64 byte emitter
  → W^X executable memory
  → CPU
```

The same validated AST can instead run through `src/interp.cpp`, which is the
correctness oracle and benchmark baseline.

## Responsibilities

| Stage | File | Owns |
|-------|------|------|
| Lexer | `src/lexer.cpp` | scanning, checked literals, source offsets |
| Parser/validator | `src/parser.cpp` | grammar, precedence, names, arity, definite declarations |
| Optimizer/codegen | `src/codegen.cpp` | folding, register/spill policy, instruction encoding, patches |
| Interpreter | `src/interp.cpp` | independent AST evaluation |
| JIT memory | `src/jitmem.cpp` | page allocation, W^X transition, lifetime |
| CLI | `src/main.cpp` | modes, persistent REPL, benchmark, error reporting |

## Backend model

The naive baseline leaves each expression in `RAX` and saves left operands with
`push`/`pop`. The optimized path folds constants, uses compact immediates, and
keeps common intermediates in caller-saved `R10` and `R11`. If the scratch pool
is exhausted, or a live value must survive a nested generated call, it spills
to the stack. Every temporary push is tracked so calls receive the alignment
required by the System V ABI.

The AST remains the boundary between frontend and backend. JIT memory is owned
by a non-copyable RAII object and is writable only during population.

See the [roadmap](04-roadmap.md) and [milestone specs](milestones/).
