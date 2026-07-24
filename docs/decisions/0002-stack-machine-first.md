# ADR 0002 — Stack-machine codegen first, register allocation later

**Status:** accepted · **Date:** 2026

## Context

The code generator must decide where intermediate values live. Either:

1. A **stack machine**: one accumulator (`RAX`), intermediates pushed/popped.
2. A **register allocator** from day one: values assigned to CPU registers,
   spilling only when necessary.

## Decision

Ship the **stack machine in milestone 0**, and introduce **register allocation
in milestone 5** as an explicit, measured improvement.

## Rationale

- The stack machine is correct for arbitrarily deep expressions in ~20 lines and
  needs no allocator, so milestone 0 can deliver the *whole pipeline* (lex →
  parse → encode → execute) rather than a half-finished backend. Getting the
  spine working end to end first is what makes every later milestone verifiable.
- Doing register allocation later makes it **measurable**: milestone 6 can
  publish a before/after benchmark on the same programs. An optimisation you
  can't quantify is a claim; one you can is evidence.
- Teaching order matters: encoding and ABI first, allocation second, is how the
  concepts build.

## Consequences

- Milestone 0's output is deliberately inefficient (`push`/`pop` around every
  binary operation). This is documented, not accidental — see
  [docs/08](../08-optimisation.md).
- Milestone 5 introduces an intermediate instruction list (needed for peephole
  work) with byte emission as the final step, a mild refactor of `codegen.cpp`.
- The public benchmark table in the README is the payoff.
