# 01 — Getting started

## Requirements

Linux or WSL2 on x86-64 with `g++` (C++17) and `make`:

```bash
sudo apt-get update && sudo apt-get install -y g++ make
```

nutjit emits System V x86-64 instructions and uses `mmap`/`mprotect`, so it
does not run natively on Windows or ARM.

## Build and use

```bash
make all
make run

./build/nutjit "2 + 3 * 4;"       # 14
./build/nutjit --dump "1 + 2;"    # bytes plus result
./build/nutjit --interp "1 + 2;"  # reference backend
./build/nutjit --file demo.nut
./build/nutjit --repl
```

The REPL preserves all successful previous inputs, including variable and
function definitions. A rejected input does not corrupt that history.

## Test and benchmark

```bash
make test
make bench
```

`make test` runs 63 behavioral cases through both the JIT and interpreter,
requires invalid programs to fail in both modes, and checks six important
machine-code encoding properties. `make bench` reports repeated medians for the
interpreter, compile time, and JIT execution.

## Inspect machine code

```bash
./build/nutjit --dump "1 + 2;" 2>&1 |
  grep -E '^[0-9a-f ]+$' | xxd -r -p > /tmp/nutjit-code.bin
objdump -D -b binary -m i386:x86-64 /tmp/nutjit-code.bin
```

See [testing and debugging](09-testing-and-debugging.md) for more.
