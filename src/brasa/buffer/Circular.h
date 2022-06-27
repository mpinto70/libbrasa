#pragma once

#include <brasa/buffer/CRC.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <type_traits>

namespace brasa {
namespace buffer {

/// Read and write head marker
struct Head {
    uint32_t offset; ///< offset from begin of buffer
    uint32_t lap;    ///< lap number in the buffer
};
static_assert(std::is_trivial_v<Head>, "TYPE must be POD");

template <typename TYPE_, uint32_t N_>
struct BufferData {
    uint8_t buffer[N_ * sizeof(TYPE_)]; ///< the data
    Head write_head;                    ///< position and lap of the next write
    Head read_head;                     ///< position and lap of the next read
    uint64_t key;                       ///< a unique key
    uint32_t crc;                       ///< CRC of the key
};

/** Circular buffer base class. It should be extended by Writer and Reader.
 * `TYPE_` is the struct that will be stored in the buffer
 * `N_` is the number of objects of type `TYPE_` that can be stored in the buffer
 */
template <typename TYPE_, uint32_t N_>
class Circular {
protected: // to allow testing and prevent use outside of the classes
    using BufferDataT = BufferData<TYPE_, N_>;

public:
    static_assert(std::is_nothrow_move_assignable_v<TYPE_>);
    static_assert(std::is_nothrow_move_constructible_v<TYPE_>);
    using TYPE = TYPE_;
    constexpr static uint32_t N = N_;
    constexpr static uint32_t BUFFER_SIZE = sizeof(BufferDataT);

    // no copies no moves
    Circular(const Circular& other) = delete;
    Circular& operator=(const Circular& other) = delete;
    Circular(Circular&& other) = delete;
    Circular& operator=(Circular&& other) = delete;

protected:
    /** Creates the object informing the buffer and the key.
     * The buffer has to be at least `BUFFER_SIZE` in length.
     */
    Circular(uint8_t* buffer, const uint64_t key) : buffer_(buffer), key_(key), crc_(crc32(key)) {
        if (not is_initialized()) {
            initialize();
        }
    }

    ~Circular() noexcept = default;

    /// Writes `data` to the buffer
    void do_write(const TYPE& data) const noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        ::memcpy(&buffer_[buffer_data->write_head.offset], &data, sizeof(TYPE));
        advance(buffer_data->write_head);
    }

    /// Reads `data` from the buffer and returns true. If there is no data in the buffer, returns false.
    bool do_read(TYPE& data) const noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        const auto write_head = buffer_data->write_head;
        auto read_head = buffer_data->read_head;

        if (read_head.offset == write_head.offset && read_head.lap == write_head.lap) {
            return false;
        }
        switch (write_head.lap - read_head.lap) {
            case 0: // same lap
                break;
            case 1: // one lap ahead
                if (write_head.offset > read_head.offset) {
                    read_head.offset = write_head.offset;
                }
                break;
            default: // more than one lap ahead
                read_head.offset = write_head.offset;
                read_head.lap = write_head.lap - 1;
        }
        ::memcpy(&data, &buffer_[read_head.offset], sizeof(TYPE));
        advance(read_head);
        buffer_data->read_head = read_head;

        return true;
    }

private:
    uint8_t* buffer_;
    const uint64_t key_;
    const uint32_t crc_;
    constexpr static uint32_t OFFSET_END_OF_DATA = sizeof(BufferDataT::buffer);

    [[nodiscard]] bool is_valid(const Head& head) const noexcept {
        return (head.offset % sizeof(TYPE) == 0) && head.offset < OFFSET_END_OF_DATA;
    }

    [[nodiscard]] bool is_valid(const Head& read_head, const Head& write_head) const noexcept {
        return is_valid(read_head) && is_valid(write_head) && read_head.lap <= write_head.lap
               && (read_head.lap != write_head.lap || read_head.offset <= write_head.offset);
    }

    [[nodiscard]] bool is_initialized() const noexcept {
        const auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        if (not is_valid(buffer_data->read_head, buffer_data->write_head)) {
            return false;
        }

        return buffer_data->key == key_ && buffer_data->crc == crc_;
    }

    void initialize() const noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        constexpr Head zero = { 0, 0 };

        buffer_data->read_head = zero;
        buffer_data->write_head = zero;
        buffer_data->key = key_;
        buffer_data->crc = crc_;
    }

    void advance(Head& head) const noexcept {
        head.offset += sizeof(TYPE);
        if (head.offset == OFFSET_END_OF_DATA) {
            head.offset = 0;
            ++head.lap;
        }
    }
};
}
}
