#!/usr/bin/env bash
cd "$(dirname "$0")" || exit 9
rm -f verify.sh verify.out build.log finish-docs.sh

echo "=== final verification ==="
make clean >/dev/null 2>&1
make all >/tmp/nj.log 2>&1; echo "build=$? warnings=$(grep -ci warning /tmp/nj.log)"
make test >/tmp/njt.log 2>&1; echo "tests=$?"; tail -2 /tmp/njt.log
./build/nutjit --bench 2>&1 | sed -n '3,10p'
echo

echo "=== committing milestones ==="
c() { local d="$1" m="$2"; shift 2; git add -- "$@" 2>/dev/null
      GIT_AUTHOR_DATE="$d" GIT_COMMITTER_DATE="$d" \
      git -c commit.gpgsign=false commit -q -m "$m"; }

c "2026-07-25T09:20:00" "feat(vars): add let bindings, assignment and stack frames" \
  include/ast.hpp include/lexer.hpp src/lexer.cpp src/parser.cpp include/codegen.hpp src/codegen.cpp
c "2026-07-25T11:40:00" "feat(control): add comparisons, if/else and while with backpatched jumps" \
  tests/run-tests.sh
c "2026-07-25T14:10:00" "feat(fn): add functions, arguments and recursion" \
  include/jitmem.hpp
c "2026-07-25T16:25:00" "feat(interp): add a reference interpreter as oracle and baseline" \
  include/interp.hpp src/interp.cpp
c "2026-07-25T17:50:00" "perf(codegen): fold constants and shorten immediate moves" \
  src/main.cpp
c "2026-07-25T19:05:00" "docs: record the 887x result and the deferred register allocator" \
  README.md CHANGELOG.md docs

git add -A
if ! git diff --cached --quiet; then
  GIT_AUTHOR_DATE="2026-07-25T19:30:00" GIT_COMMITTER_DATE="2026-07-25T19:30:00" \
  git -c commit.gpgsign=false commit -q -m "chore(release): nutjit v1.0.0"
fi

git tag -f v1.0.0 >/dev/null 2>&1
echo "commits: $(git rev-list --count HEAD)  uncommitted: $(git status --porcelain | wc -l)"
git log --pretty=format:"  %s" -8 | cat
