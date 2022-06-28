#pragma once

#include <brasa/buffer/CRC.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <utility>

namespace brasa::buffer::impl {

/// Read and write head marker
struct Head {
    uint32_t index; ///< index into data
    uint32_t lap;   ///< lap number in the data
};
static_assert(std::is_trivial_v<Head>, "Head must remain a POD");

template <typename TYPE_, uint32_t N_>
struct BufferData {
    TYPE_ data[N_];  ///< the data
    Head write_head; ///< position and lap of the next write
    Head read_head;  ///< position and lap of the next read
    uint64_t key;    ///< a unique key
    uint32_t crc;    ///< CRC of the key
};

/** Circular buffer base class. It should be extended by Writer and Reader.
 * `TYPE_` is the struct that will be stored in the data
 * `N_` is the number of objects of type `TYPE_` that can be stored in the data
 */
template <typename TYPE_, uint32_t N_>
class Circular {
protected: // to allow testing and prevent use outside of the classes
    using BufferDataT = BufferData<TYPE_, N_>;

public:
    static_assert(N_ > 1); // at least 2
    static_assert(std::is_nothrow_move_assignable_v<TYPE_>);
    static_assert(std::is_nothrow_move_constructible_v<TYPE_>);
    static_assert(std::is_copy_assignable_v<TYPE_>);
    static_assert(std::is_copy_constructible_v<TYPE_>);
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

    /// Stores `value` into `data`
    void do_write(TYPE value) noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        buffer_data->data[buffer_data->write_head.index] = std::move(value);
        advance(buffer_data->write_head);
    }

    /// Reads `value` from `data` and returns true. If there is no element in `data`, returns false.
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

    [[nodiscard]] bool is_valid(const Head& head) const noexcept { return head.index < N_; }

    [[nodiscard]] bool is_valid(const Head& read_head, const Head& write_head) const noexcept {
        return is_valid(read_head) && is_valid(write_head) && read_head.lap <= write_head.lap
               && (read_head.lap != write_head.lap || read_head.index <= write_head.index);
    }

    [[nodiscard]] bool is_initialized() const noexcept {
        const auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        return is_valid(buffer_data->read_head, buffer_data->write_head) && buffer_data->key == key_
               && buffer_data->crc == crc_;
    }

    void initialize() noexcept {
        auto buffer_data = reinterpret_cast<BufferDataT*>(buffer_);
        constexpr Head zero = { 0, 0 };

        buffer_data->read_head = zero;
        buffer_data->write_head = zero;
        buffer_data->key = key_;
        buffer_data->crc = crc_;
    }

    void advance(Head& head) noexcept {
        ++head.index;
        if (head.index == N_) {
            head.index = 0;
            ++head.lap;
        }
    }
};
}
