# 08 — Optimisation

The naive backend is retained as the benchmark baseline. The optimized backend
applies three small, inspectable improvements.

## Constant folding

Pure binary expressions with literal children become one literal AST node.
Addition, subtraction, and multiplication use explicitly defined
two's-complement wrapping. Division by zero and `INT64_MIN / -1` remain runtime
operations so the optimizer never invokes C++ undefined behavior.

## Scratch-register allocation

The optimized emitter uses caller-saved `R10` and `R11` for intermediate
values. A live scratch value is never kept across a subtree containing a call.
When both registers are busy, the emitter falls back to a tracked stack spill.
This bounded pool is simpler than a full IR/liveness allocator, but removes the
common `push`/`pop` pair and has a safe spill path.

## Instruction selection

- Nonnegative 32-bit literals use `mov eax, imm32` instead of `movabs`.
- Signed 8-bit right operands use compact immediate forms for arithmetic and
  comparisons.
- Comparisons normalize results with `setcc` and `movzx`.

`tests/check-encodings.sh` locks down representative byte sequences. Behavioral
tests independently require the JIT and interpreter to agree.

## Measurement

`make bench` uses three interpreter runs, 21 compile samples, and 21 JIT runs,
then reports the median. It also verifies every backend returns the same value
and prints naive versus optimized code size. Compile and run time are shown
separately because compilation is part of the JIT tradeoff.
