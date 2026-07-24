# Milestones

Each milestone leaves a compiler that **builds clean and passes its tests**.
Build them in order — the path is strictly linear (see the
[roadmap](../04-roadmap.md)).

| # | Milestone | State |
|---|-----------|-------|
| 0 | [Arithmetic JIT](milestone-0-arithmetic-jit.md) | ✅ done |
| 1 | [Variables](milestone-1-variables.md) | ⬜ |
| 2 | [Conditionals](milestone-2-conditionals.md) | ⬜ |
| 3 | [Loops](milestone-3-loops.md) | ⬜ |
| 4 | [Functions](milestone-4-functions.md) | ⬜ |
| 5 | [Optimisation](milestone-5-optimisation.md) | ⬜ |
| 6 | [Polish](milestone-6-polish.md) | ⬜ |

## Every milestone spec has

**Goal · Concepts · Tasks · Files · Definition of Done · References.**

## The Builder's loop (from AGENTS.md)

1. Pick the lowest-numbered unfinished milestone.
2. Implement its tasks; keep `make all` warning-free.
3. Add test cases for the new feature and make `make test` pass — the values
   must be produced by JIT-compiled code.
4. Update the concept doc, tick the DoD, update the roadmap/README/CHANGELOG.
5. Commit (`type(scope): …`), keep CI green.

## Debugging reminder

A wrong encoding crashes rather than erroring. Disassemble first:

```bash
./build/nutjit --dump "<expr>" 2>&1 | grep -E '^[0-9a-f ]+$' | xxd -r -p > /tmp/c.bin
objdump -D -b binary -m i386:x86-64 /tmp/c.bin
```
