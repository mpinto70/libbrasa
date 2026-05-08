#pragma once

/**
 * @file
 * A circular buffer of capacity @p N_ stores elements of type @p TYPE_ in a
 * flat @p BufferData block that lives in caller-supplied memory (e.g. shared
 * memory). The block is identified by a @p key / CRC pair so that an already-
 * initialised buffer can be detected and reused across process restarts.
 *
 * Lap-based head tracking allows the reader to detect and recover from
 * overruns: when the writer is more than one lap ahead the reader fast-forwards
 * to the oldest unread slot rather than reading stale data.
 */

#include <brasa/buffer/CRC.h>

#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace brasa::buffer::detail {

/**
 * Cursor that tracks the current position inside a circular buffer. Both the
 * write head and the read head are represented as a @p Head.
 * @attention `lap` wraps around after 2^32 laps, but this should not be an
 * issue in practice since each lap corresponds to `N_` writes.
 */
struct Head final {
    uint32_t index; ///< Slot index in the data array (0...N-1).
    uint32_t lap;   ///< Number of times the index has wrapped around.
};
static_assert(std::is_trivial_v<Head>, "Head must remain a POD");

/**
 * Raw memory layout of the circular buffer.
 * This struct is mapped directly onto the caller-supplied byte buffer, so its
 * layout must remain stable (no virtual functions, no padding surprises).
 * @tparam TYPE_ Element type stored in the buffer.
 * @tparam N_    Capacity in number of elements.
 */
template <typename TYPE_, uint32_t N_>
struct BufferData final {
    TYPE_ data[N_];  ///< Ring of stored elements.
    Head write_head; ///< Position and lap of the next write slot.
    Head read_head;  ///< Position and lap of the next read slot.
    uint64_t key;    ///< Unique identifier used to verify buffer ownership.
    uint32_t crc;    ///< CRC-32 of @p key, used to detect uninitialized memory.
};

/**
 * Base class for the circular buffer. Provides the core read/write logic and
 * buffer-initialisation bookkeeping. Intended to be used only through the
 * @p Writer and @p Reader subclasses.
 *
 * **Thread / process safety:** concurrent access by one writer and one reader
 * is supported provided accesses to individual slots are atomic at the hardware
 * level. No explicit synchronisation primitives are used.
 *
 * **Overrun behaviour:** if the writer is exactly one lap ahead and its index
 * has already passed the reader's index, the reader skips forward. If the
 * writer is more than one lap ahead the reader catches up to within one lap of
 * the writer, skipping all intermediate data.
 *
 * @tparam TYPE_ Element type. Must be copy- and nothrow-move constructible/assignable.
 * @tparam N_    Capacity (number of elements). Must be at least 2.
 */
template <typename TYPE_, uint32_t N_>
class Circular {
protected: // to allow testing and prevent use outside of the classes
    using BufferDataT = BufferData<TYPE_, N_>;

public:
    static_assert(N_ >= 2);
    static_assert(std::is_nothrow_move_assignable_v<TYPE_>);
    static_assert(std::is_nothrow_move_constructible_v<TYPE_>);
    static_assert(std::is_copy_assignable_v<TYPE_>);
    static_assert(std::is_copy_constructible_v<TYPE_>);
    static_assert(std::is_standard_layout_v<BufferData<TYPE_, N_>>);
    static_assert(std::is_trivially_copyable_v<BufferData<TYPE_, N_>>);

    using TYPE = TYPE_;
    /** Buffer capacity (number of elements). */
    constexpr static uint32_t N = N_;
    /** Required size of the raw byte buffer. */
    constexpr static std::size_t BUFFER_SIZE = sizeof(BufferDataT);
    static constexpr std::size_t MIN_BUFFER_SIZE = BUFFER_SIZE + alignof(BufferDataT) - 1;

    // no copies no moves
    Circular(const Circular& other) = delete;
    Circular& operator=(const Circular& other) = delete;
    Circular(Circular&& other) = delete;
    Circular& operator=(Circular&& other) = delete;

    /**
     * Returns the aligned pointer within the provided buffer.
     *
     * @param buffer Pointer to the raw memory buffer.
     * @return uint8_t* First aligned pointer within the buffer.
     */
    static uint8_t* aligned_in_buffer(void* buffer) {
        std::size_t space = MIN_BUFFER_SIZE;
        std::align(alignof(BufferDataT), BUFFER_SIZE, buffer, space);
        return static_cast<uint8_t*>(buffer);
    }

protected:
    /**
     * Constructs the circular buffer view over an existing byte buffer. If the
     * buffer is not yet initialised (key/CRC mismatch or invalid heads), it is
     * reset to an empty state.
     *
     * The view is aligned to the alignment of `BufferDataT` (possibly skipping
     * some initial bytes, that will remain untouched), so the caller must
     * ensure that the provided buffer is at least `MIN_BUFFER_SIZE` bytes long.
     *
     * @param buffer Pointer to raw memory of at least @p MIN_BUFFER_SIZE bytes.
     * @param key    Unique identifier for this buffer; used to detect whether
     *               the buffer has already been initialised.
     */
    Circular(uint8_t* buffer, const uint64_t key)
          : buffer_(aligned_in_buffer(buffer)),
            key_(key),
            crc_(crc32(key)) {
        if (not is_initialized()) {
            initialize();
        }
    }

    ~Circular() noexcept = default;

    /**
     * Writes @p value into the next available slot and advances the write head.
     * If the buffer is full the oldest unread element is silently overwritten.
     * @param value Element to store (moved into the buffer).
     */
    void do_write(TYPE value) noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        buffer_data->data[buffer_data->write_head.index] = std::move(value);
        advance(buffer_data->write_head);
    }

    /**
     * Reads the next available element into @p value and advances the read head.
     * If the writer has lapped the reader, the read head is fast-forwarded to
     * avoid returning stale data.
     * @param[out] value Receives the element on success.
     * @return @c true if an element was read; @c false if the buffer is empty.
     */
    bool do_read(TYPE& value) noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        const auto write_head = buffer_data->write_head;
        auto read_head = buffer_data->read_head;

        if (read_head.index == write_head.index && read_head.lap == write_head.lap) {
            return false;
        }
        switch (write_head.lap - read_head.lap) {
            case 0: // same lap
                break;
            case 1: // one lap ahead
                if (write_head.index > read_head.index) {
                    read_head.index = write_head.index;
                }
                break;
            default: // more than one lap ahead
                read_head.index = write_head.index;
                read_head.lap = write_head.lap - 1;
        }
        value = std::move(buffer_data->data[read_head.index]);
        advance(read_head);
        buffer_data->read_head = read_head;

        return true;
    }

private:
    uint8_t* buffer_;
    const uint64_t key_;
    const uint32_t crc_;

    /** Returns @c true if @p head holds a valid slot index (< N_). */
    [[nodiscard]] bool is_valid(const Head& head) const noexcept { return head.index < N_; }

    /**
     * Returns @c true if the read/write head pair describes a consistent state:
     * both indices are in range, the read lap does not exceed the write lap,
     * and within the same lap the read index does not exceed the write index.
     */
    [[nodiscard]] bool is_valid(const Head& read_head, const Head& write_head) const noexcept {
        return is_valid(read_head) && is_valid(write_head) && read_head.lap <= write_head.lap
               && (read_head.lap != write_head.lap || read_head.index <= write_head.index);
    }

    /**
     * Returns @c true if the buffer's key, CRC, and head positions are all
     * consistent with this instance — i.e. the buffer was previously
     * initialised by a @p Circular with the same @p key.
     */
    [[nodiscard]] bool is_initialized() const noexcept {
        const auto buffer_data = reinterpret_cast<const BufferDataT*>(buffer_);
        return is_valid(buffer_data->read_head, buffer_data->write_head) && buffer_data->key == key_
               && buffer_data->crc == crc_;
    }

    /** Resets the buffer to an empty state and stamps it with @p key_ and @p crc_. */
    void initialize() noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        constexpr Head zero = { 0, 0 };

        buffer_data->read_head = zero;
        buffer_data->write_head = zero;
        buffer_data->key = key_;
        buffer_data->crc = crc_;
    }

    /** Advances @p head to the next slot, wrapping around and incrementing the lap counter. */
    void advance(Head& head) noexcept {
        ++head.index;
        if (head.index == N_) {
            head.index = 0;
            ++head.lap;
        }
    }
};
} // namespace brasa::buffer::detail
