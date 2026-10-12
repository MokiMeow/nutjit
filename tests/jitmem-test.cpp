/* Keep W^X code mapping and entry-point bounds independently testable. */
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "jitmem.hpp"

int main() {
    // mov eax,42; ret. Valid System V x86-64 function with no arguments.
    const std::vector<uint8_t> code = {
        0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3,
    };
    const JitBuffer buffer(code);
    if (buffer.code_size() != code.size() || buffer.size() < code.size())
        return 1;
    if (buffer.run() != 42)
        return 2;

    // Reject both the logical end and page padding, never execute them.
    for (size_t offset : {code.size(), buffer.size()}) {
        try {
            (void)buffer.run(offset);
            return 3;
        } catch (const std::out_of_range &) {
            // Expected.
        }
    }

    try {
        const JitBuffer empty(std::vector<uint8_t>{});
        (void)empty;
        return 4;
    } catch (const std::runtime_error &) {
        // Expected.
    }

    std::puts("ok jitmem W^X entry-boundary tests");
    return 0;
}
