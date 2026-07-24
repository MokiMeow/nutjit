#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

/* Stage 4: turn a byte vector into something the CPU will execute.
 *
 * Machine code can't run from ordinary heap memory — the pages must be marked
 * executable. JitBuffer maps pages writable, copies the code in, then flips
 * them to read+execute (W^X: never writable and executable at once). */
class JitBuffer {
public:
    using Fn = int64_t (*)();

    explicit JitBuffer(const std::vector<uint8_t> &code);
    ~JitBuffer();

    JitBuffer(const JitBuffer &) = delete;
    JitBuffer &operator=(const JitBuffer &) = delete;

    /* Call the compiled code. */
    int64_t run() const { return reinterpret_cast<Fn>(memory_)(); }

    const uint8_t *bytes() const { return static_cast<const uint8_t *>(memory_); }
    size_t size() const { return size_; }

private:
    void  *memory_ = nullptr;
    size_t size_ = 0;
};
