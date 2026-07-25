/* nutjit — compile a program to x86-64 machine code and run it.
 *
 *   nutjit "let x = 5; x * 2;"      compile and print the result
 *   nutjit --dump "1+2"             also print the machine code
 *   nutjit --interp "..."           run through the reference interpreter
 *   nutjit --file program.nut       run a file
 *   nutjit --repl                   interactive
 *   nutjit --bench                  JIT vs interpreter on fib(30)
 */

#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "codegen.hpp"
#include "interp.hpp"
#include "jitmem.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace {

using Clock = std::chrono::steady_clock;

void dump_code(const CompiledProgram &program) {
    std::fprintf(stderr, "nutjit: %zu bytes of machine code (entry at +%zu)\n",
                 program.code.size(), program.entry_offset);
    for (size_t i = 0; i < program.code.size(); i++)
        std::fprintf(stderr, "%02x%s", program.code[i],
                     (i + 1) % 16 == 0 || i + 1 == program.code.size() ? "\n" : " ");
}

int64_t jit_run(const std::string &source, bool dump, bool optimise = true) {
    const auto tokens = tokenize(source);
    const NodePtr ast = parse(tokens);
    const CompiledProgram program = codegen(*ast, optimise);
    if (dump)
        dump_code(program);
    const JitBuffer buffer(program.code);
    return buffer.run(program.entry_offset);
}

int64_t interp_run(const std::string &source) {
    const auto tokens = tokenize(source);
    const NodePtr ast = parse(tokens);
    return interpret(*ast);
}

double seconds_since(Clock::time_point start) {
    return std::chrono::duration<double>(Clock::now() - start).count();
}

const char *FIB_SOURCE =
    "fn fib(n) {\n"
    "  if (n < 2) { return n; }\n"
    "  return fib(n - 1) + fib(n - 2);\n"
    "}\n"
    "fib(30);\n";

int benchmark() {
    std::printf("nutjit benchmark — fib(30), single-threaded\n\n");

    const auto tokens = tokenize(FIB_SOURCE);
    const NodePtr ast = parse(tokens);

    // Interpreter
    auto start = Clock::now();
    const int64_t interp_result = interpret(*ast);
    const double interp_time = seconds_since(start);

    // JIT, unoptimised
    start = Clock::now();
    const CompiledProgram naive = codegen(*ast, false);
    const double naive_compile = seconds_since(start);
    const JitBuffer naive_buffer(naive.code);
    start = Clock::now();
    const int64_t naive_result = naive_buffer.run(naive.entry_offset);
    const double naive_time = seconds_since(start);

    // JIT, optimised
    start = Clock::now();
    const CompiledProgram opt = codegen(*ast, true);
    const double opt_compile = seconds_since(start);
    const JitBuffer opt_buffer(opt.code);
    start = Clock::now();
    const int64_t opt_result = opt_buffer.run(opt.entry_offset);
    const double opt_time = seconds_since(start);

    if (interp_result != naive_result || interp_result != opt_result) {
        std::fprintf(stderr,
                     "FAIL: back ends disagree (interp=%lld naive=%lld opt=%lld)\n",
                     (long long)interp_result, (long long)naive_result,
                     (long long)opt_result);
        return 1;
    }

    std::printf("  fib(30) = %lld  (all three back ends agree)\n\n",
                (long long)interp_result);
    std::printf("  %-22s %10s %12s %10s\n", "implementation", "run (ms)", "compile (ms)", "speedup");
    std::printf("  %-22s %10.2f %12s %9.1fx\n", "tree-walking interp",
                interp_time * 1e3, "-", 1.0);
    std::printf("  %-22s %10.2f %12.3f %9.1fx\n", "nutjit (naive)",
                naive_time * 1e3, naive_compile * 1e3, interp_time / naive_time);
    std::printf("  %-22s %10.2f %12.3f %9.1fx\n", "nutjit (optimised)",
                opt_time * 1e3, opt_compile * 1e3, interp_time / opt_time);
    std::printf("\n  code size: naive %zu bytes, optimised %zu bytes\n",
                naive.code.size(), opt.code.size());
    return 0;
}

int repl() {
    std::printf("nutjit REPL — expressions and definitions are compiled to machine code.\n");
    std::printf("Definitions persist across lines. Ctrl-D to exit.\n\n");

    std::string accumulated;   // function definitions carry over
    std::string line;
    while (true) {
        std::printf("nutjit> ");
        std::fflush(stdout);
        if (!std::getline(std::cin, line))
            break;
        if (line.empty())
            continue;

        // A definition is remembered; anything else is evaluated against the
        // definitions seen so far.
        const bool is_definition = line.rfind("fn ", 0) == 0;
        std::string program = accumulated + (is_definition ? line : line);
        if (!is_definition && program.find(';') == std::string::npos)
            program += ";";

        try {
            const int64_t result = jit_run(program, false);
            if (is_definition) {
                accumulated += line + "\n";
                std::printf("defined\n");
            } else {
                std::printf("%lld\n", (long long)result);
            }
        } catch (const std::exception &error) {
            std::printf("error: %s\n", error.what());
        }
    }
    std::printf("\n");
    return 0;
}

std::string read_file(const std::string &path) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("cannot open '" + path + "'");
    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}

} // namespace

int main(int argc, char **argv) {
    bool dump = false, use_interp = false;
    std::string source;

    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--dump")        dump = true;
        else if (arg == "--interp") use_interp = true;
        else if (arg == "--bench")  return benchmark();
        else if (arg == "--repl")   return repl();
        else if (arg == "--file") {
            if (++i >= argc) { std::fprintf(stderr, "--file needs a path\n"); return 2; }
            try { source = read_file(argv[i]); }
            catch (const std::exception &e) { std::fprintf(stderr, "nutjit: %s\n", e.what()); return 1; }
        }
        else source = arg;
    }

    if (source.empty() && !std::getline(std::cin, source)) {
        std::fprintf(stderr,
                     "usage: nutjit [--dump] [--interp] \"<program>\"\n"
                     "       nutjit --file <path> | --repl | --bench\n");
        return 2;
    }

    try {
        const int64_t result = use_interp ? interp_run(source) : jit_run(source, dump);
        std::printf("%lld\n", (long long)result);
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "nutjit: %s\n", error.what());
        return 1;
    }
}
