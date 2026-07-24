/* nutjit — compile an expression to x86-64 machine code and run it.
 *
 *   nutjit "2 + 3 * 4"     compile and print the result
 *   nutjit --dump "1+2"    also print the machine code bytes
 *   nutjit                 read the expression from stdin
 */

#include <cstdio>
#include <iostream>
#include <string>

#include "codegen.hpp"
#include "jitmem.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace {

void dump_code(const std::vector<uint8_t> &code) {
    std::fprintf(stderr, "nutjit: %zu bytes of machine code\n", code.size());
    for (size_t i = 0; i < code.size(); i++) {
        std::fprintf(stderr, "%02x%s", code[i],
                     (i + 1) % 16 == 0 || i + 1 == code.size() ? "\n" : " ");
    }
}

} // namespace

int main(int argc, char **argv) {
    bool dump = false;
    std::string source;

    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--dump")
            dump = true;
        else
            source = arg;
    }

    if (source.empty() && !std::getline(std::cin, source)) {
        std::fprintf(stderr, "usage: nutjit [--dump] \"<expression>\"\n");
        return 2;
    }

    try {
        const auto tokens = tokenize(source);
        const NodePtr ast = parse(tokens);
        const auto code = codegen(*ast);

        if (dump)
            dump_code(code);

        const JitBuffer buffer(code);
        std::printf("%lld\n", static_cast<long long>(buffer.run()));
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "nutjit: %s\n", error.what());
        return 1;
    }
}
