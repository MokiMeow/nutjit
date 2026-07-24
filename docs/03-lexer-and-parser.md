# 03 — Lexer & parser (the front end)

## The lexer

`src/lexer.cpp` is a hand-written scanner: a loop over characters that skips
whitespace, accumulates digit runs into an `int64_t`, and maps single characters
to operator tokens. Every token carries its **source offset**, which is what
makes error messages point at the right place.

```
"2 + 30"  ->  [Number(2)@0, Plus@2, Number(30)@4, End@6]
```

Unknown characters throw `std::runtime_error` naming the character and offset.

Milestone 1 adds identifiers and keywords: scan an alphanumeric run, then look
it up in a keyword table (`let`, later `if`, `while`, `fn`) — the standard
approach that avoids a separate keyword scanner.

## The parser

`src/parser.cpp` is **recursive descent**: one function per precedence level.

```
expr   := term (('+' | '-') term)*
term   := factor (('*' | '/') factor)*
factor := NUMBER | '(' expr ')' | '-' factor
```

Two properties fall out of this shape for free:

- **Precedence** — `term` is *called by* `expr`, so multiplication binds tighter
  than addition. `2+3*4` parses as `2+(3*4)` without any precedence table.
- **Left associativity** — each level loops (`while` over its operators),
  folding the accumulated node into the left-hand side. `100-10-5` becomes
  `(100-10)-5`, which is why it evaluates to 85 rather than 95.

**Unary minus** is desugared: `-x` is parsed as `0 - x`, so the code generator
never needs a unary case. Small trick, one less code path.

## Why recursive descent

- The code *is* the grammar — you can read one against the other.
- Error messages are natural: each function knows what it expected.
- No parser generator, no build step, no generated code to debug. For a language
  this size, table-driven parsing would add machinery without adding capability.

## Growing the grammar

Later milestones extend it to statements and declarations:

```
program   := statement*
statement := 'let' IDENT '=' expr ';' | 'if' '(' expr ')' block ('else' block)?
           | 'while' '(' expr ')' block | 'fn' IDENT '(' params ')' block
           | expr ';'
```

Keep the one-function-per-rule discipline: it's what keeps a hand-written parser
readable as the language grows.
