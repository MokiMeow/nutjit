# Milestone 4: Functions ✅ (done)

**Goal:** user-defined functions with arguments and recursion: enough to run
`fib(30)` as native code. This is the milestone that makes nutjit a real
compiler.

## Concepts

The System V argument registers, `call`/`ret`, 16-byte stack alignment at call
sites, compiling multiple functions into one code buffer, and resolving calls
to functions defined later.

## Tasks

- [x] **Parser**: `fn name(a, b) { … }` definitions and `name(arg, …)` calls;
      a program is a list of functions plus a top-level expression to evaluate.
- [x] **Codegen: function bodies**: each function gets a prologue/epilogue
      (milestone 1) and its own environment. Parameters arrive in
      `RDI, RSI, RDX, RCX, R8, R9`; spill them to `[rbp-N]` slots on entry so
      the rest of codegen treats them exactly like locals.
- [x] **Codegen: calls**: evaluate arguments, move them into the argument
      registers, `call rel32` (`E8` + 4 bytes), result arrives in `RAX`.
- [x] **Alignment**: keep `RSP` 16-byte aligned at every `call` (frame sizes a
      multiple of 16). Misalignment shows up as a crash inside libc, not at your
      call site: see [docs/07](../07-calling-convention.md).
- [x] **Forward references**: record each `call`'s displacement position and the
      callee name; after all functions are emitted, patch every call with the
      real offset. (Same patch helper as milestone 2.)
- [x] Emit all functions into **one** buffer so `rel32` distances are valid.
- [x] **Tests**: `fn add(a,b){ a+b; } add(2,3);` → 5; a recursive factorial;
      `fib(10)` → 55; `fib(30)` → 832040 (this becomes the benchmark program).

## Files

`src/parser.cpp`, `include/ast.hpp`, `src/codegen.cpp` (multi-function emission
+ call patching), `src/main.cpp`, `tests/run-tests.sh`, `docs/07`.

## Definition of Done

- [x] `fib(30)` returns **832040** from JIT-compiled machine code.
- [x] Recursion works to a reasonable depth without corrupting the stack.
- [x] A function may call one defined later in the file (forward reference).
- [x] `make all` warning-free; `make test` green.

## Notes

If recursion crashes at depth, suspect (a) stack alignment, or (b) a callee-saved
register being clobbered: nutjit's stack machine sticks to `RAX`/`RCX` for
exactly this reason; if you start using `RBX`/`R12`–`R15`, you must save and
restore them.

**Next:** [Milestone 5: Optimisation](milestone-5-optimisation.md).
