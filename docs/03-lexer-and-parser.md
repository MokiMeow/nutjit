# 03 — Lexer, parser, and validation

## Lexer

`src/lexer.cpp` is a hand-written scanner. Every token carries a byte offset.
Integer accumulation is checked before every digit is added, so literals above
`INT64_MAX` fail at their source position instead of overflowing C++.

Identifiers are scanned once and classified through the keyword table (`let`,
`if`, `else`, `while`, `fn`, and `return`).

## Recursive-descent parser

One function implements each precedence level:

```
comparison := sum (('<' | '>' | '<=' | '>=' | '==' | '!=') sum)*
sum        := product (('+' | '-') product)*
product    := unary (('*' | '/') unary)*
unary      := '-' unary | primary
primary    := NUMBER | IDENT | call | '(' comparison ')'
```

Programs add blocks, `let`, assignment, conditionals, loops, functions, and
returns. Looping within a precedence level gives left associativity; calling
the next tighter level gives precedence without a table.

## Semantic validation

Parsing is followed by a pass that collects function signatures and checks:

- duplicate function names and duplicate parameters,
- unknown functions and wrong call arity,
- unknown variables and assignments,
- `return` outside a function,
- use of a variable only when it is definitely declared.

An `if` declaration survives after the conditional only when both branches
declare it. A declaration made only inside a `while` does not survive because
the loop may execute zero times. These rules prevent the JIT from loading an
uninitialized frame slot and ensure the interpreter and JIT reject the same
invalid program.
