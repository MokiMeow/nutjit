# 09 — Testing & debugging

A JIT fails differently from ordinary code: a wrong byte doesn't raise an
exception, it crashes the process — or worse, returns a plausible wrong number.

## The test suite is the contract

`tests/run-tests.sh` runs the binary over expressions with known answers. Every
value is produced by **executing generated machine code**, so a passing suite is
direct evidence the encodings are right. It covers:

- each operator, precedence, and left associativity,
- parentheses and unary minus,
- a 64-bit result (`1000000 * 1000000`) — catches 32-bit truncation,
- syntax errors that must be *rejected* (an accepted bad program is a bug too).

Add cases with every milestone; a feature without tests isn't done.

## Disassemble what you emitted

The highest-value debugging tool:

```bash
./build/nutjit --dump "1 + 2" 2>&1 | grep -E '^[0-9a-f ]+$' | xxd -r -p > /tmp/c.bin
objdump -D -b binary -m i386:x86-64 /tmp/c.bin
```

If `objdump` prints the instructions you meant, the bytes are right. `(bad)`
means a malformed encoding. This turns "why did it segfault" into a five-second
check.

## Debug the generated code with GDB

```bash
gdb --args ./build/nutjit "1 + 2"
(gdb) break JitBuffer::run
(gdb) run
(gdb) x/20i $rip      # disassemble at the current instruction pointer
(gdb) stepi           # step one machine instruction
(gdb) info registers rax rcx
```

Because the code is generated, GDB has no symbols for it — `x/i` and `stepi`
are how you walk it.

## Common failures

| Symptom | Likely cause |
|---------|--------------|
| SIGSEGV on `run()` | missing `ret`, or a malformed encoding |
| SIGILL | bytes aren't a valid instruction — disassemble and compare |
| SIGFPE on division | `idiv` without `cqo` (garbage in RDX), or divide by zero |
| Wrong result, no crash | operand order (`sub rax, rcx` vs `rcx, rax`) or endianness of an immediate |
| Truncated large values | emitting a 32-bit form where 64-bit was needed |
| `jit: mprotect failed` | environment denies `PROT_EXEC` |
| Crash only when calling | 16-byte stack misalignment at the `call` (M4) |

## Sanitizers

For front-end bugs (parser, AST lifetimes), build with sanitizers:

```bash
make clean && make CXXFLAGS="-std=c++17 -Wall -Wextra -g -fsanitize=address,undefined -Iinclude"
```

Note ASan and JIT-executed memory interact awkwardly — use sanitizers to chase
lexer/parser/AST issues, and `objdump`/GDB for codegen issues.

## CI

CI builds with zero warnings and runs `make test` on every push. Because nutjit
emits x86-64, CI must run on an x86-64 runner (`ubuntu-latest` is).
