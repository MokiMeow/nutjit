# 07 — Calling convention (System V AMD64)

Generated code has to agree with the host program about registers and the
stack. On Linux/WSL2 that contract is **System V AMD64**.

## The rules we depend on

- **Return value**: in `RAX`. This is why every nutjit expression leaves its
  result there — `JitBuffer::run()` casts to `int64_t(*)()` and reads `RAX`.
- **Integer arguments**: `RDI, RSI, RDX, RCX, R8, R9`, in order (milestone 4).
- **Callee-saved**: `RBX, RBP, R12–R15` — if generated code uses them, it must
  save and restore them. Our stack machine deliberately uses only `RAX`/`RCX`
  (both caller-saved) so milestone 0 needs no saving at all.
- **Caller-saved**: `RAX, RCX, RDX, RSI, RDI, R8–R11` — free to clobber.
- **Stack alignment**: `RSP` must be **16-byte aligned at the point of a
  `call`**. On entry to a function, `RSP+8` is aligned (the `call` pushed an
  8-byte return address).

## Why alignment doesn't bite in milestone 0 — but will in milestone 4

Milestone 0's code never issues a `call`, and its `push`/`pop` are perfectly
balanced, so alignment is irrelevant. The moment generated code calls something
(another JIT-compiled function, or a runtime helper), misalignment causes
crashes that look random — typically inside SSE instructions in libc, which
require 16-byte-aligned memory.

The fix is the standard prologue, which milestone 1 introduces for locals and
milestone 4 relies on:

```
push rbp          ; 55
mov  rbp, rsp     ; 48 89 E5
sub  rsp, N       ; 48 83 EC N   (N a multiple of 16)
...
leave             ; C9   (mov rsp, rbp ; pop rbp)
ret               ; C3
```

With `N` a multiple of 16, `RSP` stays aligned across the whole body.

## Locals (milestone 1)

Local variables live at negative offsets from `RBP`:

```
mov [rbp-8],  rax     ; 48 89 45 F8      store local #1
mov rax, [rbp-16]     ; 48 8B 45 F0      load  local #2
```

The environment built by the parser maps each name to its slot index; codegen
turns the index into the offset `-(8 * index)`.

## Windows note

Windows x64 uses a different convention (`RCX, RDX, R8, R9`, plus a 32-byte
shadow space). nutjit targets System V only — another reason it runs under
WSL2 rather than natively on Windows.

## References

- [System V AMD64 ABI](https://gitlab.com/x86-psABIs/x86-64-ABI)
- [OSDev — System V ABI](https://wiki.osdev.org/System_V_ABI)
