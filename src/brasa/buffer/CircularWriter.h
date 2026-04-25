#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa::buffer {

/**
 * Write-only view over a circular buffer stored in caller-supplied memory.
 *
 * `CircularWriter` and `CircularReader` are the two public access points for
 * the same underlying `detail::Circular` buffer. A typical usage pattern is to
 * have one process (or thread) hold a `CircularWriter` and another hold a
 * `CircularReader` over the same raw memory region (e.g. a shared-memory
 * segment).
 *
 * **Overrun behaviour**: when the buffer is full the oldest unread element is
 * silently overwritten. The reader detects and recovers from overruns
 * automatically -- see `CircularReader` and `detail::Circular` for details.
 *
 * **Thread / process safety**: concurrent access by exactly one writer and one
 * reader is supported. Multiple concurrent writers are *not* supported.
 *
 * @tparam TYPE_ Element type to store. Must satisfy the constraints of
 *               `detail::Circular` (copy/nothrow-move constructible and
 *               assignable, standard-layout, trivially copyable).
 * @tparam N_    Buffer capacity in number of elements. Must be at least 2.
 *
 * @see CircularReader
 * @see detail::Circular
 */
template <typename TYPE_, size_t N_>
class CircularWriter : public detail::Circular<TYPE_, N_> {
public:
    using Base = detail::Circular<TYPE_, N_>;
    using Base::MIN_BUFFER_SIZE;
    using Base::N;
    using Base::TYPE;

    /**
     * Constructs a writer over an existing raw byte buffer.
     *
     * If the buffer has not yet been initialised (key/CRC mismatch or invalid
     * head positions), it is reset to an empty state. Otherwise the existing
     * content is left intact, allowing the writer to resume after a restart.
     *
     * @param buffer Pointer to raw memory of at least `BUFFER_SIZE` bytes,
     *               aligned to `alignof(detail::BufferData<TYPE_, N_>)`.
     * @param key    Unique identifier for this buffer instance, used to detect
     *               whether the buffer has already been initialised by a
     *               compatible writer/reader pair.
     * @throws std::invalid_argument if @p buffer does not meet the alignment
     *         requirement.
     */
    CircularWriter(uint8_t* buffer, uint64_t key) : Base(buffer, key) {}

    /**
     * Writes @p value into the next available slot and advances the write head.
     *
     * If the buffer is full the oldest unread element is silently overwritten.
     * The operation is always successful and never blocks.
     *
     * @param value Element to store. The value is copied into the buffer.
     */
    void write(const TYPE_& value) noexcept { Base::do_write(value); }
};
} // namespace brasa::buffer
