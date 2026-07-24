#!/usr/bin/env bash
# Expression test suite: each case compiles to machine code, runs on the CPU,
# and must produce the expected value. This is the milestone's proof — the
# results come from executing JIT-compiled code, not from an interpreter.
set -uo pipefail

BIN=${1:-build/nutjit}
pass=0
fail=0

check() {
    local expr="$1" want="$2" got
    got=$("$BIN" "$expr" 2>/dev/null)
    if [[ "$got" == "$want" ]]; then
        printf '  ok    %-28s = %s\n' "$expr" "$got"
        pass=$((pass + 1))
    else
        printf '  FAIL  %-28s = %s (want %s)\n' "$expr" "${got:-<error>}" "$want"
        fail=$((fail + 1))
    fi
}

check_error() {
    local expr="$1"
    if "$BIN" "$expr" >/dev/null 2>&1; then
        printf '  FAIL  %-28s should have been rejected\n' "$expr"
        fail=$((fail + 1))
    else
        printf '  ok    %-28s rejected\n' "$expr"
        pass=$((pass + 1))
    fi
}

echo "----- nutjit expression tests -----"
check "1"                        1
check "1 + 2"                    3
check "7 - 9"                    -2
check "6 * 7"                    42
check "84 / 2"                   42
check "2 + 3 * 4"                14      # precedence
check "(2 + 3) * 4"              20      # parentheses
check "100 - 10 - 5"             85      # left associativity
check "100 / 10 / 5"             2       # left associativity
check "-5 + 8"                   3       # unary minus
check "-(3 * 4)"                 -12
check "2 + 3 * (10 - 4) / 2"     11
check "1000000 * 1000000"        1000000000000  # 64-bit result

check_error "2 +"
check_error "(1 + 2"
check_error "1 $ 2"

echo "-----------------------------------"
echo "passed: $pass   failed: $fail"
[[ $fail -eq 0 ]]
