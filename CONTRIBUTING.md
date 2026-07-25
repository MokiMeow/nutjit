# Contributing to nutjit

This is primarily a learning/portfolio project, but clean contributions are
welcome.

## Before you start

- Read [AGENTS.md](AGENTS.md) — the operating manual, which applies to humans
  too.
- Skim [docs/00-overview.md](docs/00-overview.md), the
  [roadmap](docs/04-roadmap.md), and
  [docs/05-x86-codegen.md](docs/05-x86-codegen.md).
- You need x86-64 Linux or WSL2 with `g++` (C++17).

## Workflow

1. Pick the lowest-numbered unfinished milestone in
   [docs/milestones/](docs/milestones/), or an open issue.
2. Branch from `main`: `git checkout -b milestone-2-conditionals`.
3. Implement it. A clean `-Werror` build and `make test` must pass at every
   commit.
4. Add differential cases for behavior and byte-level assertions when an exact
   encoding is part of the contract.
5. Update the relevant doc and tick the Definition of Done.
6. Open a PR into `main`; CI must be green.

## Commit style

`type(scope): outcome` in the imperative, lower case. Types: `feat`, `fix`,
`docs`, `refactor`, `build`, `chore`, `test`. No AI/co-author trailers.

Example: `feat(codegen): emit conditional jumps with backpatching`.

## Code style

See §4 of [AGENTS.md](AGENTS.md). C++17, 4-space indent, `snake_case` functions,
`PascalCase` types, trailing `_` on private members. Every emitted instruction
gets a comment with its byte encoding.

## Reporting issues

Include the expression, the expected and actual result, and — for crashes — the
`--dump` output run through `objdump` (see
[docs/09](docs/09-testing-and-debugging.md)).
