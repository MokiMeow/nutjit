/* Executable memory for the JIT.
 *
 * mmap anonymous pages as read+write, copy the machine code in, then mprotect
 * them to read+execute. Keeping write and execute permissions separate in time
 * (W^X) is what modern OSes expect; some harden against ever allowing both. */

#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/mman.h>

#include "jitmem.hpp"

JitBuffer::JitBuffer(const std::vector<uint8_t> &code) {
    if (code.empty())
        throw std::runtime_error("jit: refusing to map an empty code buffer");

    const long page = sysconf(_SC_PAGESIZE);
    if (page <= 0)
        throw std::runtime_error("jit: could not determine the page size");

    const size_t page_size = static_cast<size_t>(page);
    size_ = ((code.size() + page_size - 1) / page_size) * page_size;

    memory_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_ == MAP_FAILED) {
        memory_ = nullptr;
        throw std::runtime_error(std::string("jit: mmap failed: ") + std::strerror(errno));
    }

    std::memcpy(memory_, code.data(), code.size());

    if (mprotect(memory_, size_, PROT_READ | PROT_EXEC) != 0) {
        const std::string message = std::string("jit: mprotect failed: ")
                                    + std::strerror(errno);
        munmap(memory_, size_);
        memory_ = nullptr;
        throw std::runtime_error(message);
    }
}

JitBuffer::~JitBuffer() {
    if (memory_ != nullptr)
        munmap(memory_, size_);
}
