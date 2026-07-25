# nutjit — build system
#
# Quick start (WSL2 / Linux):
#   make run     # JIT-compile a sample program and dump its machine code
#   make test    # the differential test suite (JIT vs interpreter)
#   make bench   # fib(30): interpreter vs JIT
#   make repl    # interactive

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

BUILD := build
BIN   := $(BUILD)/nutjit

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRC))

.PHONY: all run test bench repl clean

all: $(BIN)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Compile a sample program to machine code, dump the bytes, and run it.
# Statements end in ';' — the folded expression collapses to a single mov.
run: $(BIN)
	@$(BIN) --dump "2 + 3 * (10 - 4) / 2;"
	@echo "--- a program with a function, a loop and recursion ---"
	@$(BIN) "fn fib(n) { if (n < 2) { return n; } return fib(n-1) + fib(n-2); } fib(20);"

test: $(BIN)
	@bash tests/run-tests.sh $(BIN)
	@bash tests/check-encodings.sh $(BIN)

bench: $(BIN)
	@$(BIN) --bench

repl: $(BIN)
	@$(BIN) --repl

clean:
	rm -rf $(BUILD)
