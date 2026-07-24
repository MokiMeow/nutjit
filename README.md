<h1 align="center">nutjit</h1>

<p align="center">
  <em>A small language with a JIT compiler that emits real x86-64 machine code
  into executable memory and jumps to it — no LLVM, no interpreter fallback.</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/arch-x86__64-blue" alt="x86_64">
  <img src="https://img.shields.io/badge/lang-C%2B%2B17-orange" alt="C++17">
  <img src="https://img.shields.io/badge/backend-hand--rolled%20codegen-red" alt="hand-rolled codegen">
  <img src="https://img.shields.io/badge/license-MIT-lightgrey" alt="MIT">
</p>

---

## What this is

Most "write a language" projects stop at a tree-walking interpreter, or hand
code generation to LLVM. nutjit does neither: it takes source text through a
lexer, a recursive-descent parser, and its own **x86-64 code generator**, writes
the resulting bytes into `mmap`'d executable pages, casts them to a function
pointer, and **calls them**. The number it prints was computed by machine code
this program wrote at runtime.

```
source ──▶ lexer ──▶ parser ──▶ AST ──▶ codegen ──▶ bytes ──▶ mmap+mprotect ──▶ CALL
 "2+3*4"    tokens    grammar          x86-64        48 b8 …    R|X pages       14
```

## See it work

```bash
$ make run
nutjit: 86 bytes of machine code
48 b8 02 00 00 00 00 00 00 00 50 48 b8 03 00 00
00 00 00 00 00 50 48 b8 0a 00 00 00 00 00 00 00
...
11
```

Those bytes are a real function: `48 b8 …` is `movabs rax, imm64`, `50` is
`push rax`, `48 01 c8` is `add rax, rcx`, and `c3` is `ret`.

## Why it is interesting (the depth on show)

- **A compiler pipeline you can read** — hand-written lexer, recursive-descent
  parser with precedence falling out of the grammar, and a typed AST.
  ([docs/03-lexer-and-parser.md](docs/03-lexer-and-parser.md))
- **Machine-code generation by hand** — REX prefixes, ModR/M bytes, and why
  `idiv` needs `cqo` first. Every instruction is emitted byte by byte.
  ([docs/05-x86-codegen.md](docs/05-x86-codegen.md))
- **W^X executable memory** — mapping pages writable, then flipping them to
  read+execute before jumping in.
  ([docs/06-jit-memory.md](docs/06-jit-memory.md))
- **The calling convention, for real** — System V AMD64: arguments in registers,
  the return value in RAX, stack alignment that matters once you make calls.
  ([docs/07-calling-convention.md](docs/07-calling-convention.md))

## Quick start (WSL2 / Linux, $0)

```bash
sudo apt-get install -y g++ make    # one time
make run                            # compile a sample expression and run it
make test                           # the expression test suite
./build/nutjit "2 + 3 * 4"          # -> 14
./build/nutjit --dump "1 + 2"       # show the machine code it generated
```

## Status

Milestone 0 (arithmetic → machine code → execute) is **done** — the repo builds
and JITs today, with 16 passing tests. The road to a real little language is in
[docs/04-roadmap.md](docs/04-roadmap.md).

| # | Milestone | State |
|---|-----------|-------|
| 0 | Arithmetic JIT (lex → parse → x86-64 → run) | ✅ done |
| 1 | Variables, `let`, stack frames | ⬜ |
| 2 | Comparisons + `if`/`else` (jumps and patching) | ⬜ |
| 3 | Loops (`while`) and backward jumps | ⬜ |
| 4 | Functions, arguments, recursion | ⬜ |
| 5 | Register allocation + peephole optimisation | ⬜ |
| 6 | REPL, benchmarks, CI, `v1.0.0` | ⬜ |

The endgame benchmark: run `fib(30)` through the JIT and against a
tree-walking interpreter of the same AST, and publish the speedup.

## Repository layout

```
nutjit/
├── src/          # lexer, parser, codegen, jit memory, main
├── include/      # headers (ast, lexer, parser, codegen, jitmem)
├── tests/        # expression test suite
├── docs/         # architecture, codegen, roadmap, milestones, ADRs
└── Makefile      # all / run / test / clean
```

## Requirements

Linux or WSL2 on x86-64, with `g++` (C++17). The JIT emits x86-64 instructions
and uses `mmap`/`mprotect`, so it is deliberately platform-specific — see
[ADR 0003](docs/decisions/0003-x86-only.md).

## License

MIT — see [LICENSE](LICENSE).
