#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa::buffer {

/**
 * Read-only view over a circular buffer stored in caller-supplied memory.
 *
 * `CircularReader` and `CircularWriter` are the two public access points for
 * the same underlying `detail::Circular` buffer. A typical usage pattern is to
 * have one process (or thread) hold a `CircularWriter` and another hold a
 * `CircularReader` over the same raw memory region (e.g. a shared-memory
 * segment).
 *
 * **Overrun behaviour**: if the writer has advanced more than one full lap
 * ahead of the reader, the read head is fast-forwarded so that only recent data
 * is returned -- stale slots are skipped silently.
 *
 * **Thread / process safety**: concurrent access by exactly one writer and one
 * reader is supported. Multiple concurrent readers are *not* supported.
 *
 * @tparam TYPE_ Element type to read. Must satisfy the constraints of
 *               `detail::Circular` (copy/nothrow-move constructible and
 *               assignable, standard-layout, trivially copyable).
 * @tparam N_    Buffer capacity in number of elements. Must be at least 2.
 *
 * @see CircularWriter
 * @see detail::Circular
 */
template <typename TYPE_, size_t N_>
class CircularReader : public detail::Circular<TYPE_, N_> {
public:
    using Base = detail::Circular<TYPE_, N_>;
    using Base::MIN_BUFFER_SIZE;
    using Base::N;
    using Base::TYPE;

    /**
     * Constructs a reader over an existing raw byte buffer.
     *
     * If the buffer has not yet been initialised (key/CRC mismatch or invalid
     * head positions), it is reset to an empty state. Otherwise the existing
     * content is left intact, allowing the reader to resume after a restart.
     *
     * @param buffer Pointer to raw memory of at least `BUFFER_SIZE` bytes,
     *               aligned to `alignof(detail::BufferData<TYPE_, N_>)`.
     * @param key    Unique identifier for this buffer instance, used to detect
     *               whether the buffer has already been initialised by a
     *               compatible writer/reader pair.
     * @throws std::invalid_argument if @p buffer does not meet the alignment
     *         requirement.
     */
    CircularReader(uint8_t* buffer, uint64_t key) : Base(buffer, key) {}

    /**
     * Reads the next available element into @p value and advances the read head.
     *
     * If the writer has lapped the reader, the read head is fast-forwarded
     * before reading so that only recent (non-stale) data is returned.
     *
     * @param[out] value Receives the element on success; unchanged on failure.
     * @return `true` if an element was read; `false` if the buffer is empty.
     */
    bool read(TYPE_& value) noexcept { return Base::do_read(value); }
};
} // namespace brasa::buffer
