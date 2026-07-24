# 02 — Architecture

Four stages, each a separate translation unit with one job. Data flows one way;
no stage knows about the stages after it.

```
┌────────────┐  std::string   ┌────────────┐  vector<Token> ┌────────────┐
│  lexer.cpp │───────────────▶│ parser.cpp │───────────────▶│    AST     │
└────────────┘                └────────────┘                └─────┬──────┘
                                                                  │ NodePtr
                                                                  ▼
┌────────────────────┐  int64_t   ┌────────────┐  vector<uint8_t> ┌──────────┐
│  the CPU (call it) │◀───────────│ jitmem.cpp │◀─────────────────│codegen.cpp│
└────────────────────┘            └────────────┘                  └──────────┘
```

## Stage responsibilities

| Stage | File | In → Out | Owns |
|-------|------|----------|------|
| Lexer | `src/lexer.cpp` | text → `vector<Token>` | character classification, number scanning, source offsets |
| Parser | `src/parser.cpp` | tokens → `NodePtr` | grammar, precedence, associativity, syntax errors |
| Codegen | `src/codegen.cpp` | AST → `vector<uint8_t>` | instruction selection and byte encoding |
| JIT memory | `src/jitmem.cpp` | bytes → callable | `mmap`/`mprotect`, W^X, lifetime |

`src/main.cpp` wires them together and handles CLI flags and error reporting.

## Why this split matters

- **The AST is the contract.** The parser doesn't know x86 exists; the code
  generator doesn't know what the source looked like. That's what makes a second
  backend (ARM64) a stretch goal rather than a rewrite.
- **`JitBuffer` owns the memory.** RAII: the pages are unmapped in the
  destructor, and the class is non-copyable so ownership is unambiguous.
- **Errors are exceptions with offsets.** Lexer and parser throw
  `std::runtime_error` including the byte offset; `main` catches and prints. No
  stage calls `exit()`.

## The code generator's model (milestone 0)

A **stack machine**: every expression leaves its value in `RAX`. A binary node
compiles as

```
  <compile left>          ; result in rax
  push rax
  <compile right>         ; result in rax
  mov rcx, rax            ; right operand
  pop rax                 ; left operand
  <one instruction>       ; add/sub/imul/idiv rax, rcx
```

This needs no register allocator and generalises to any expression depth, at the
cost of redundant pushes and pops — exactly the inefficiency milestone 5
replaces with real register allocation, so the improvement is measurable.

## How the language grows

| Milestone | Lexer | Parser | Codegen | Runtime |
|-----------|-------|--------|---------|---------|
| 1 variables | identifiers, `let`, `=` | statements, environment | `rbp` frame, `[rbp-N]` slots | — |
| 2 conditionals | `< > == !=` | `if`/`else` | `cmp`/`setcc`, `jcc` + backpatching | — |
| 3 loops | `while` | loop node | backward jumps | — |
| 4 functions | `fn`, `,` | defs, calls | arg registers, `call`/`ret`, alignment | multiple buffers |
| 5 optimisation | — | — | register allocator, folding, peephole | — |

See the [roadmap](04-roadmap.md) and the [milestones](milestones/).
