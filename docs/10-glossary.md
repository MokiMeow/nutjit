# 10: Glossary

- **AST (abstract syntax tree)**: the parsed shape of a program; the contract
  between the front end and the code generator.
- **Backpatching**: emitting a jump with a placeholder offset, recording where
  it is, and filling in the real distance once the target address is known.
- **Caller-saved / callee-saved**: the side of a call responsible for
  preserving a register (see [docs/07](07-calling-convention.md)).
- **`cqo`**: sign-extends `RAX` into `RDX:RAX`; required before `idiv`.
- **Constant folding**: evaluating constant subexpressions at compile time.
- **Immediate**: a literal value encoded inside an instruction, little-endian.
- **JIT (just-in-time) compiler**: compiles to machine code at run time and
  executes it, rather than ahead of time or interpreting.
- **Left associativity**: `a-b-c` grouping as `(a-b)-c`; produced by looping
  in each precedence level of a recursive-descent parser.
- **Little-endian**: least-significant byte first; how x86 stores immediates.
- **ModR/M**: the instruction byte encoding operand registers/addressing.
- **`mmap` / `mprotect`**: allocate pages / change their protection; how
  executable memory is obtained.
- **Opcode**: the byte(s) identifying the instruction.
- **Peephole optimisation**: rewriting short local instruction patterns.
- **Precedence**: the rule that determines which operator binds tighter,
  encoded by the grammar's shape.
- **Prologue / epilogue**: `push rbp; mov rbp, rsp; sub rsp, N` … `leave; ret`;
  sets up and tears down a stack frame.
- **Recursive descent**: a parser with one function per grammar rule.
- **Register allocation**: assigning values to CPU registers instead of memory.
- **REX prefix**: `0x40`-`0x4F`; `REX.W` (`0x48`) selects 64-bit operands.
- **Sethi–Ullman numbering**: computing the minimum registers an expression
  tree needs, so the deeper subtree is compiled first.
- **SIGILL / SIGSEGV / SIGFPE**: illegal instruction / bad memory access /
  arithmetic fault; the three ways bad codegen announces itself.
- **Stack machine**: a code-generation strategy where every value passes
  through one accumulator and the stack; simple, slow, allocator-free.
- **System V AMD64 ABI**: the Linux calling convention nutjit obeys.
- **Tokens**: the lexer's output: typed lexical units with source offsets.
- **Tree-walking interpreter**: executes the AST directly; nutjit's milestone-6
  benchmark baseline, not its execution path.
- **W^X (write xor execute)**: never mapping memory writable and executable at
  once; the reason `JitBuffer` uses `mprotect` after copying.
