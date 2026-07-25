#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

/* Stage 4: turn a byte vector into something the CPU will execute.
 *
 * Machine code cannot run from ordinary heap memory — the pages must be marked
 * executable. JitBuffer maps pages writable, copies the code in, then flips
 * them to read+execute (W^X: never writable and executable at once). */
class JitBuffer {
public:
    using Fn = int64_t (*)();

    explicit JitBuffer(const std::vector<uint8_t> &code);
    ~JitBuffer();

    JitBuffer(const JitBuffer &) = delete;
    JitBuffer &operator=(const JitBuffer &) = delete;

    /* Call the compiled code, entering at `entry_offset` bytes into the buffer.
     * Several functions share one buffer so their `call rel32` displacements
     * are valid, so the entry point is not necessarily offset 0. */
    int64_t run(size_t entry_offset = 0) const {
        auto base = static_cast<const uint8_t *>(memory_);
        auto fn = reinterpret_cast<Fn>(const_cast<uint8_t *>(base + entry_offset));
        return fn();
    }

    const uint8_t *bytes() const { return static_cast<const uint8_t *>(memory_); }
    size_t size() const { return size_; }

private:
    void  *memory_ = nullptr;
    size_t size_ = 0;
};
