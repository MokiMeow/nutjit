#!/usr/bin/env bash
# Byte-exact checks for the emitter. Behavioural differential tests catch wrong
# answers; these checks catch accidental changes to the actual x86-64 encoding.
set -euo pipefail

bin=${1:-build/nutjit}
test_dir=$(mktemp -d)
trap 'rm -rf -- "$test_dir"' EXIT

dump_hex() {
    local source=$1
    "$bin" --dump "$source" >"$test_dir/stdout" 2>"$test_dir/stderr"
    awk '/^[0-9a-f][0-9a-f]( |$)/ {
             gsub(/ /, "");
             printf "%s", $0
         }
         END { print "" }' "$test_dir/stderr"
}

expect_exact() {
    local name=$1 source=$2 expected=$3 actual
    actual=$(dump_hex "$source")
    if [[ "$actual" != "$expected" ]]; then
        echo "[FAIL] $name encoding" >&2
        echo "want: $expected" >&2
        echo " got: $actual" >&2
        exit 1
    fi
    echo "[ok] $name encoding"
}

expect_contains() {
    local name=$1 source=$2 expected=$3 actual
    actual=$(dump_hex "$source")
    if [[ "$actual" != *"$expected"* ]]; then
        echo "[FAIL] $name missing bytes $expected" >&2
        echo " got: $actual" >&2
        exit 1
    fi
    echo "[ok] $name encoding"
}

expect_exact \
    constant-fold \
    "2 + 3 * 4 - 1;" \
    "554889e54881ec00000000b800000000b80d000000c9c3"

expect_exact \
    scratch-register \
    "let x = 2; let y = 3; x + y;" \
    "554889e54881ec10000000b800000000b802000000488985f8ffffffb803000000488985f0ffffff488b85f8ffffff4989c2488b85f0ffffff4903c2c9c3"

expect_contains \
    immediate-subtract \
    "let x = 10; x - 1;" \
    "4883e801"

alignment_source="fn id(x) { return x; } let y = 3; y + id(4);"
expect_contains nested-call-align-down "$alignment_source" "4883ec08e8"
expect_contains nested-call-align-up "$alignment_source" "4883c408"

echo "[ok] encodings"
