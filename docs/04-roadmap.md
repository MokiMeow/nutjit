# 04 — Roadmap

**Complete.** From "JITs arithmetic" to "compiles a small language with
recursive functions to native code, 887× faster than interpreting it."

## The plan, as built

| # | Milestone | What was built | What it taught |
|---|-----------|----------------|----------------|
| 0 | **Arithmetic JIT** ✅ | lexer, parser, x86-64 codegen, executable memory | the whole pipeline, instruction encoding, W^X |
| 1 | **Variables** ✅ | `let`, assignment, an environment, stack frames | prologue/epilogue, locals at `[rbp-8n]` |
| 2 | **Conditionals** ✅ | `< > <= >= == !=`, `if`/`else` | `cmp`/`setcc`/`movzx`, `jcc`, **backpatching** |
| 3 | **Loops** ✅ | `while` | backward jumps, whose displacement is known immediately |
| 4 | **Functions** ✅ | definitions, calls, args, recursion | System V arg registers, `call`/`ret`, 16-byte alignment |
| 5 | **Optimisation** ✅ | constant folding, short-`mov` peephole, interpreter baseline | measuring an optimisation instead of asserting it |
| 6 | **Polish** ✅ | REPL, benchmark, CI, `v1.0.0` | honest reporting |

## The headline result

| implementation | fib(30) run | compile | speedup |
|---|---|---|---|
| tree-walking interpreter | 5075.75 ms | — | 1.0× |
| nutjit (naive codegen) | 5.72 ms | 0.008 ms | **887.1×** |
| nutjit (optimised) | 5.78 ms | 0.033 ms | 877.5× |

Code size: 200 bytes naive → **170 bytes** optimised.

### Reading this honestly

The optimised row is **not faster** than the naive one on this benchmark, and
that is the correct result: `fib` contains no constant subexpressions, so
folding has nothing to fold, and the peephole change (`mov eax, imm32` instead
of `movabs rax, imm64`) makes the code *smaller*, not hotter. The gain shows up
where you would expect it:

```
$ nutjit --dump "2 + 3 * 4 - 1;"
b8 0d 00 00 00        # mov eax, 13 — the whole expression, folded to one instruction
```

Reporting a flat row rather than dressing it up is the point. The real,
enormous win is the JIT itself: 887× over interpretation, for 8 microseconds of
compile time.

## Definition of Done — met

nutjit compiles a program with variables, `if`/`else`, `while`, and recursive
functions to x86-64 machine code at runtime and executes it correctly —
demonstrated by `fib(30)` returning 832040 — with a published benchmark against
a tree-walking interpreter of the same AST, a REPL, green CI, and a `v1.0.0`
tag.

## Where the remaining performance is

Being honest about what was *not* done: the code generator is still a stack
machine, so every binary operation round-trips through `push`/`pop`. Real
register allocation (Sethi–Ullman ordering over a pool of caller-saved
registers) is the next big win and is left as a stretch goal rather than
claimed.

## Stretch goals (after v1.0.0)

- Register allocation to remove the `push`/`pop` traffic.
- An **ARM64 backend** behind the same AST — the strongest proof the front end
  and code generator are genuinely separate. (Note: ARM64 needs an explicit
  instruction-cache flush after writing code; x86-64 does not.)
- Strings and a small heap.
- An SSA IR with linear-scan allocation.
- A tracing tier that re-optimises hot loops.
