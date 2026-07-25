#!/usr/bin/env bash
# Every value here is computed by executing JIT-compiled machine code.
#
# `check` additionally runs the same program through the reference interpreter
# and requires both back ends to agree — a differential test. That is what
# catches a codegen bug that produces a plausible but wrong number, which is the
# failure mode a plain expected-value test misses.
set -uo pipefail

BIN=${1:-build/nutjit}
pass=0
fail=0

check() {
    local src="$1" want="$2" jit interp
    jit=$("$BIN" "$src" 2>/dev/null)
    interp=$("$BIN" --interp "$src" 2>/dev/null)
    if [[ "$jit" != "$want" ]]; then
        printf '  FAIL  %-46s jit=%s want=%s\n' "$src" "${jit:-<error>}" "$want"
        fail=$((fail + 1))
    elif [[ "$interp" != "$want" ]]; then
        printf '  FAIL  %-46s interp=%s want=%s (back ends disagree)\n' \
               "$src" "${interp:-<error>}" "$want"
        fail=$((fail + 1))
    else
        printf '  ok    %-46s = %s\n' "$src" "$jit"
        pass=$((pass + 1))
    fi
}

check_error() {
    local src="$1"
    if "$BIN" "$src" >/dev/null 2>&1; then
        printf '  FAIL  %-46s should have been rejected\n' "$src"
        fail=$((fail + 1))
    else
        printf '  ok    %-46s rejected\n' "$src"
        pass=$((pass + 1))
    fi
}

echo "----- arithmetic (milestone 0) -----"
check "1;"                                   1
check "1 + 2;"                               3
check "7 - 9;"                               -2
check "6 * 7;"                               42
check "84 / 2;"                              42
check "2 + 3 * 4;"                           14
check "(2 + 3) * 4;"                         20
check "100 - 10 - 5;"                        85
check "100 / 10 / 5;"                        2
check "-5 + 8;"                              3
check "-(3 * 4);"                            -12
check "1000000 * 1000000;"                   1000000000000

echo "----- variables (milestone 1) -----"
check "let x = 5; x;"                        5
check "let x = 5; x * 2;"                    10
check "let a = 3; let b = 4; a * b + 1;"     13
check "let x = 1; x = 42; x;"                42
check "let x = 2; let y = x * 3; y - x;"     4
check "let x = 1; x = x + 1; x = x + 1; x;"  3

echo "----- conditionals (milestone 2) -----"
check "1 < 2;"                               1
check "2 < 1;"                               0
check "3 > 2;"                               1
check "2 <= 2;"                              1
check "2 >= 3;"                              0
check "5 == 5;"                              1
check "5 != 5;"                              0
check "if (1 < 2) { 10; } else { 20; }"      10
check "if (2 < 1) { 10; } else { 20; }"      20
check "let x = 7; if (x > 5) { x * 2; } else { 0; }"   14
check "if (1) { if (1) { 5; } else { 6; } } else { 7; }" 5
check "if (0) { 1; }"                        0

echo "----- loops (milestone 3) -----"
check "let i = 0; while (i < 5) { i = i + 1; } i;"                       5
check "let i = 0; let s = 0; while (i < 5) { s = s + i; i = i + 1; } s;" 10
check "let i = 0; while (i < 0) { i = i + 1; } i;"                       0
check "let n = 0; let i = 0; while (i < 3) { let j = 0; while (j < 3) { n = n + 1; j = j + 1; } i = i + 1; } n;" 9
check "let i = 1; let f = 1; while (i <= 10) { f = f * i; i = i + 1; } f;" 3628800

echo "----- functions (milestone 4) -----"
check "fn add(a, b) { a + b; } add(2, 3);"                          5
check "fn sq(x) { x * x; } sq(9);"                                  81
check "fn id(x) { return x; } id(7);"                               7
check "fn f(a,b,c,d,e,g) { a+b+c+d+e+g; } f(1,2,3,4,5,6);"          21
check "fn fact(n) { if (n < 2) { return 1; } return n * fact(n - 1); } fact(10);" 3628800
check "fn fib(n) { if (n < 2) { return n; } return fib(n-1) + fib(n-2); } fib(20);" 6765
check "fn outer(x) { return inner(x) + 1; } fn inner(x) { return x * 2; } outer(5);" 11
check "fn add(a,b){a+b;} fn mul(a,b){a*b;} mul(add(1,2), add(3,4));"  21
check "fn f(n) { let acc = 0; let i = 0; while (i < n) { acc = acc + i; i = i + 1; } return acc; } f(100);" 4950

echo "----- errors -----"
check_error "2 +;"
check_error "(1 + 2;"
check_error "1 \$ 2;"
check_error "undefined_var;"
check_error "no_such_fn(1);"
check_error "let x = 1"
check_error "if (1) { 2; "

echo "-------------------------------------"
echo "passed: $pass   failed: $fail"
[[ $fail -eq 0 ]]
