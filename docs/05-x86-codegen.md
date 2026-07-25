# 05 — x86-64 code generation

This is the heart of the project: turning an AST into bytes the CPU will run.

## Anatomy of an x86-64 instruction

```
[prefixes] [REX] [opcode] [ModR/M] [SIB] [displacement] [immediate]
```

- **REX** — the 64-bit prefix. `0x48` is `REX.W` = "operate on 64-bit
  registers." Almost every instruction we emit starts with it.
- **ModR/M** — encodes the operands. `0xC8` for `add rax, rcx` means
  "register-direct, source RCX, destination RAX."
- **Immediate** — literal bytes, **little-endian** (least significant first),
  which is why `movabs rax, 2` is `48 b8 02 00 00 00 00 00 00 00`.

## The instructions milestone 0 emits

| Assembly | Bytes | Why |
|---|---|---|
| `movabs rax, imm64` | `48 B8` + 8 bytes | load a literal (full 64-bit form so big constants work) |
| `push rax` | `50` | save the left operand |
| `pop rax` | `58` | restore the left operand |
| `mov rcx, rax` | `48 89 C1` | move the right operand aside |
| `add rax, rcx` | `48 01 C8` | |
| `sub rax, rcx` | `48 29 C8` | operand order matters: `rax - rcx` |
| `imul rax, rcx` | `48 0F AF C1` | two-byte opcode (`0F` escape) |
| `cqo` | `48 99` | sign-extend RAX into RDX:RAX |
| `idiv rcx` | `48 F7 F9` | divides RDX:RAX; **needs `cqo` first** |
| `ret` | `C3` | return to the host |

### The `idiv` trap

`idiv` divides the 128-bit value in `RDX:RAX`, not just `RAX`. If you forget
`cqo`, whatever garbage is in `RDX` becomes the high half — so `84 / 2` might
return nonsense, or the CPU raises `#DE` and your process dies. This is the
single most common first bug in a hand-rolled x86 backend.

## The baseline and optimized strategies

Every expression leaves its result in `RAX`:

```
emit(Number n)   -> movabs rax, n
emit(Binary op)  -> emit(left); push rax
                    emit(right); mov rcx, rax; pop rax
                    <op> rax, rcx
```

The naive benchmark path retains this stack-machine strategy. The optimized
path uses `R10` and `R11` for common intermediates, compact signed immediates,
and tracked stack spills when the pool is exhausted or a value must survive a
nested call.

## Verifying what you emitted

Never trust an encoding you haven't checked:

```bash
./build/nutjit --dump "1 + 2" 2>&1 | grep -E '^[0-9a-f ]+$' | xxd -r -p > /tmp/c.bin
objdump -D -b binary -m i386:x86-64 /tmp/c.bin
```

If `objdump` shows the instructions you intended, the encoding is right. If it
shows `(bad)`, you have a byte wrong.

## What later milestones add

- **M1**: `push rbp; mov rbp, rsp; sub rsp, N` prologue, `mov [rbp-N], rax`
  local slots, `leave; ret` epilogue.
- **M2**: `cmp rax, rcx` + `setcc al` + `movzx rax, al`; `jcc rel32` with
  **backpatching** — emit the jump with a placeholder offset, record its
  position, fill in the real distance once the target is known.
- **M3**: backward jumps (the offset is known immediately, so no patching).
- **M4**: `call rel32`, arguments in `RDI, RSI, RDX, RCX, R8, R9`, and 16-byte
  stack alignment at the call site.
- **M5**: AST folding, compact immediates, scratch-register allocation, and a
  safe spill path.

## References

- [felixcloutier.com/x86](https://www.felixcloutier.com/x86/) — per-instruction
  encodings.
- Intel SDM Volume 2 — the authority.
- [OSDev — X86-64 Instruction Encoding](https://wiki.osdev.org/X86-64_Instruction_Encoding)
