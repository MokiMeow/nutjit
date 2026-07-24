# 01 — Getting started

## Requirements

Linux or WSL2 on **x86-64**, with `g++` (C++17) and `make`:

```bash
sudo apt-get update && sudo apt-get install -y g++ make
```

nutjit emits x86-64 instructions and uses `mmap`/`mprotect`, so it will not run
on ARM or Windows natively — use WSL2 on Windows.

## Build and run

```bash
make run
```

That builds `build/nutjit`, compiles the sample expression to machine code,
dumps the bytes, and executes them:

```
nutjit: 86 bytes of machine code
48 b8 02 00 00 00 00 00 00 00 50 48 b8 03 00 00
...
11
```

## Use it

```bash
./build/nutjit "2 + 3 * 4"        # 14
./build/nutjit "(2 + 3) * 4"      # 20
./build/nutjit --dump "1 + 2"     # show the generated machine code
echo "6 * 7" | ./build/nutjit     # reads stdin when given no expression
```

## Test

```bash
make test
```

Runs `tests/run-tests.sh`: arithmetic, precedence, associativity, unary minus,
64-bit results, and rejected syntax errors. Every expected value is produced by
executing JIT-compiled code.

## Inspect the machine code

The `--dump` output is real x86-64. To disassemble it and check your encodings:

```bash
./build/nutjit --dump "1 + 2" 2>&1 | grep -E '^[0-9a-f ]+$' | xxd -r -p > /tmp/code.bin
objdump -D -b binary -m i386:x86-64 /tmp/code.bin
```

See [docs/09](09-testing-and-debugging.md) for the full debugging workflow.

## Make targets

| Command | Purpose |
|---------|---------|
| `make all` | Build `build/nutjit`. |
| `make run` | Build, dump, and run a sample expression. |
| `make test` | Run the expression test suite. |
| `make clean` | Delete `build/`. |

## Troubleshooting

- **`g++: command not found`** — install the toolchain (above).
- **SIGSEGV / SIGILL when running** — a bad instruction encoding in
  `src/codegen.cpp`. Disassemble the `--dump` output and compare against
  [felixcloutier.com/x86](https://www.felixcloutier.com/x86/).
- **`jit: mprotect failed`** — a hardened environment refusing `PROT_EXEC`.
  Uncommon on WSL2/desktop Linux; see [docs/06](06-jit-memory.md).
