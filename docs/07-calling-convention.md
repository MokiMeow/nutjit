# 07: Calling convention (System V AMD64)

Generated functions follow the Linux/WSL2 System V AMD64 ABI.

- Results return in `RAX`.
- The first six integer arguments use `RDI`, `RSI`, `RDX`, `RCX`, `R8`, `R9`.
- `RAX`, `RCX`, `RDX`, `RSI`, `RDI`, and `R8`–`R11` are caller-saved.
- `RBX`, `RBP`, and `R12`–`R15` are callee-saved.
- `RSP` must be 16-byte aligned immediately before a `call`.

nutjit uses `R10` and `R11` for optimized expression temporaries only when no
nested call can clobber the live value.

## Frames and call alignment

Each function establishes an `RBP` frame and reserves a multiple of 16 bytes
for locals:

```
push rbp
mov  rbp, rsp
sub  rsp, N        ; N is a multiple of 16
```

That aligns the local-frame base, but expression spills can still move `RSP` by
eight bytes. The emitter therefore tracks outstanding temporary bytes. Before
a generated call it inserts `sub rsp, 8` when needed and restores it with
`add rsp, 8` afterward. This is why nested calls remain aligned even inside a
binary expression.

Locals occupy negative offsets from `RBP`; `leave; ret` restores the caller.
Codegen asserts that each completed function has a balanced temporary stack.

Windows x64 has different argument registers and shadow-space rules. nutjit
targets System V only.

Reference: [System V AMD64 ABI](https://gitlab.com/x86-psABIs/x86-64-ABI).
