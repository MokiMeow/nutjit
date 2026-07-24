# 08 — Optimisation

*Milestone 5.* Until then the code generator is deliberately naive — that's the
baseline the optimisations are measured against.

## What's wrong with the stack machine

For `2 + 3`, milestone 0 emits:

```
movabs rax, 2
push   rax
movabs rax, 3
mov    rcx, rax
pop    rax
add    rax, rcx
```

Six instructions, two memory round-trips, for something a compiler would emit as
`mov eax, 5`. The waste is structural: every operand goes through `RAX`, and
every intermediate spills to the stack.

## The three optimisations

### 1. Constant folding (AST level)

Fold `Binary(op, Number, Number)` into a single `Number` before codegen. `2+3*4`
becomes `Number(14)` — the whole expression compiles to one `movabs`. Cheap to
implement, dramatic on constant-heavy code, and it's a pure AST→AST pass so it
can't break the backend.

Watch out: don't fold division by zero at compile time — leave it to run time
(or report it as an error deliberately).

### 2. Register allocation

Give the code generator a small pool of scratch registers (`RAX, RCX, RDX, RSI,
RDI, R8–R11` are all caller-saved and free) instead of one accumulator plus the
stack. For an expression tree, a simple approach:

- Track the next free register; compile the left side into register *r*, the
  right into *r+1*, and emit `op r, r+1`.
- Spill to the stack only when the tree is deeper than the register pool
  (**Sethi–Ullman numbering** tells you exactly when — compile the deeper
  subtree first to minimise spills).

This removes almost every `push`/`pop` in realistic expressions.

### 3. Peephole

A post-pass over the emitted instruction list, rewriting local patterns:

- `push rax; pop rax` → nothing.
- `mov rcx, rax; pop rax` where the pop overwrites → reorder/remove.
- `movabs rax, imm` where `imm` fits in 32 bits → `mov eax, imm32`
  (`B8` + 4 bytes) — 5 bytes instead of 10, and the zero-extension is free.

To do peephole work you need an intermediate list of *instructions* rather than
raw bytes — which is why milestone 5 introduces one, with byte emission as the
final step.

## Measuring it (the point)

The optimisations only count if they're measured. Milestone 6 publishes:

| implementation | `fib(30)` |
|---|---|
| tree-walking interpreter | ~X ms |
| nutjit M0-style codegen | ~Y ms |
| nutjit optimised | ~Z ms |

Benchmark honestly: same AST, same machine, several runs, report the median, and
say what the compile time was as well as the run time (JIT compile time is part
of the cost — that's the real trade-off a JIT makes).
