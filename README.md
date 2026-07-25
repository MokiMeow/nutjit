<h1 align="center">nutjit</h1>

<p align="center">
  <em>A small language with a JIT compiler that emits real x86-64 machine code
  into executable memory and jumps to it — no LLVM, no interpreter in the
  execution path, no dependencies.</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/arch-x86__64-blue" alt="x86_64">
  <img src="https://img.shields.io/badge/lang-C%2B%2B17-orange" alt="C++17">
  <img src="https://img.shields.io/badge/backend-hand--rolled%20codegen-red" alt="hand-rolled codegen">
  <img src="https://img.shields.io/badge/release-v1.0.0-brightgreen" alt="v1.0.0">
  <img src="https://img.shields.io/badge/license-MIT-lightgrey" alt="MIT">
</p>

---

## What this is

Most "write a language" projects stop at a tree-walking interpreter, or hand
code generation to LLVM. nutjit does neither: source goes through a lexer, a
recursive-descent parser, and its own **x86-64 code generator**; the resulting
bytes are written into `mmap`'d executable pages, cast to a function pointer,
and **called**. The number it prints was computed by machine code this program
wrote at runtime.

```
source ──▶ lexer ──▶ parser ──▶ AST ──▶ codegen ──▶ bytes ──▶ mmap+mprotect ──▶ CALL
"fib(30)"   tokens    grammar          x86-64        55 48 89…   R|X pages      832040
```

## The result

```
$ nutjit --bench
nutjit benchmark — fib(30), single-threaded

  fib(30) = 832040  (all three back ends agree)

  implementation           run (ms) compile (ms)    speedup
  tree-walking interp       5075.75            -       1.0x
  nutjit (naive)               5.72        0.008     887.1x
  nutjit (optimised)           5.78        0.033     877.5x

  code size: naive 200 bytes, optimised 170 bytes
```

**887× faster than interpreting the same AST**, and the whole compile takes
**8 microseconds** — that is the JIT trade in one table.

Two honest notes on it. The optimised and naive rows are the same speed here
because `fib` has no constant subexpressions to fold; folding shows up as
**code size** (200 → 170 bytes), and on constant-heavy code it is dramatic:

```
$ nutjit --dump "2 + 3 * 4 - 1;"
...  b8 0d 00 00 00  ...      # mov eax, 13 — the entire expression, one instruction
```

## The language

```rust
fn fib(n) {
  if (n < 2) { return n; }
  return fib(n - 1) + fib(n - 2);
}

let total = 0;
let i = 0;
while (i < 10) {
  total = total + fib(i);
  i = i + 1;
}
total;
```

Integers, `let` bindings and assignment, `+ - * /`, comparisons
(`< > <= >= == !=`), `if`/`else`, `while`, and functions with up to six
parameters and full recursion.

## Why it is interesting (the depth on show)

- **A compiler pipeline you can read** — hand-written lexer, recursive-descent
  parser where precedence falls out of the grammar's shape, and a typed AST.
  ([docs/03-lexer-and-parser.md](docs/03-lexer-and-parser.md))
- **Machine-code generation by hand** — REX prefixes, ModR/M bytes, `cqo`
  before `idiv`, `setcc`+`movzx` for comparisons, and **jump backpatching**:
  emit a placeholder displacement, record it, fill in the distance once the
  target is known. ([docs/05-x86-codegen.md](docs/05-x86-codegen.md))
- **Real stack frames and the System V ABI** — locals at `[rbp-8n]`, arguments
  in `RDI/RSI/RDX/RCX/R8/R9`, frames rounded to 16 bytes so `RSP` stays aligned
  at every call. Verified by 2000-deep recursion.
  ([docs/07-calling-convention.md](docs/07-calling-convention.md))
- **W^X executable memory** — pages mapped writable, filled, then flipped to
  read+execute before the jump.
  ([docs/06-jit-memory.md](docs/06-jit-memory.md))
- **A second back end as the oracle** — a tree-walking interpreter over the same
  AST. Every test runs *both* and requires them to agree, which is the only
  practical way to catch codegen that produces a plausible wrong number.

## Quick start

```bash
sudo apt-get install -y g++ make

make run                                  # compile a sample and dump its machine code
make test                                 # 51 differential tests
make bench                                # the table above

./build/nutjit "let x = 5; x * 2;"        # 10
./build/nutjit --dump "1 + 2;"            # show the generated machine code
./build/nutjit --interp "1 + 2;"          # run through the reference interpreter
./build/nutjit --file program.nut         # run a file
./build/nutjit --repl                     # interactive; definitions persist
```

## Status — complete

| # | Milestone | State |
|---|-----------|-------|
| 0 | Arithmetic JIT (lex → parse → x86-64 → run) | ✅ done |
| 1 | Variables, `let`, stack frames | ✅ done |
| 2 | Comparisons + `if`/`else` (jump backpatching) | ✅ done |
| 3 | Loops (`while`, backward jumps) | ✅ done |
| 4 | Functions, arguments, recursion | ✅ done |
| 5 | Constant folding, peephole, interpreter baseline | ✅ done |
| 6 | REPL, benchmarks, CI, `v1.0.0` | ✅ done |

**51/51 tests pass**, build is warning-free, and every test value is produced by
executing JIT-compiled machine code *and* cross-checked against the interpreter.

## Verifying the output yourself

Never trust an encoding you have not checked:

```bash
./build/nutjit --dump "1 + 2;" 2>&1 | grep -E '^[0-9a-f ]+$' | xxd -r -p > /tmp/c.bin
objdump -D -b binary -m i386:x86-64 /tmp/c.bin
```

## Repository layout

```
nutjit/
├── src/          # lexer, parser, codegen, jit memory, interpreter, cli
├── include/      # headers
├── tests/        # differential test suite
├── docs/         # architecture, codegen, roadmap, milestones, ADRs
└── Makefile      # all / run / test / bench / clean
```

## Requirements

Linux or WSL2 on **x86-64** with `g++` (C++17). The JIT emits x86-64 and uses
`mmap`/`mprotect`, so it is deliberately platform-specific — see
[ADR 0003](docs/decisions/0003-x86-only.md).

## License

MIT — see [LICENSE](LICENSE).
