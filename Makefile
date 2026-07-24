# nutjit — build system
#
# Quick start (WSL2 / Linux):
#   make run     # build and JIT-compile a sample expression
#   make test    # run the expression test suite

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

BUILD := build
BIN   := $(BUILD)/nutjit

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SRC))

.PHONY: all run test clean

all: $(BIN)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Compile a sample expression to machine code, dump it, and run it.
run: $(BIN)
	@$(BIN) --dump "2 + 3 * (10 - 4) / 2"

test: $(BIN)
	@bash tests/run-tests.sh $(BIN)

clean:
	rm -rf $(BUILD)
