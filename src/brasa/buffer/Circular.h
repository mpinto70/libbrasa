#pragma once

#include "brasa/buffer/CRC.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <type_traits>

namespace brasa {
namespace buffer {

struct Head {
    uint32_t offset;
    uint32_t lap;
};

template <typename TYPE_, uint32_t N_>
class Circular {
public:
    static_assert(std::is_pod<TYPE_>::value, "TYPE must be POD");
    using TYPE = TYPE_;
    constexpr static uint32_t N = N_;
    constexpr static uint32_t OFFSET_BEGIN_OF_DATA = 0;
    constexpr static uint32_t OFFSET_END_OF_DATA = OFFSET_BEGIN_OF_DATA + N * sizeof(TYPE);
    constexpr static uint32_t OFFSET_READ_HEAD = OFFSET_END_OF_DATA;
    constexpr static uint32_t OFFSET_WRITE_HEAD = OFFSET_READ_HEAD + sizeof(Head);
    constexpr static uint32_t OFFSET_KEY = OFFSET_WRITE_HEAD + sizeof(Head);
    constexpr static uint32_t OFFSET_CRC = OFFSET_KEY + sizeof(uint64_t);
    constexpr static uint32_t BUFFER_SIZE = OFFSET_CRC + sizeof(uint32_t);

    // no copies no moves
    Circular(const Circular& other) = delete;
    Circular& operator=(const Circular& other) = delete;

protected:
    Circular(uint8_t* buffer, const uint64_t key)
          : buffer_(buffer),
            key_(key),
            crc_(crc(key)) {
        if (not is_initialized()) {
            initialize();
        }
    }

    void do_write(const TYPE& data) const {
        const auto write_head = reinterpret_cast<Head*>(&buffer_[OFFSET_WRITE_HEAD]);
        ::memcpy(&buffer_[write_head->offset], &data, sizeof(TYPE));
        advance(*write_head);
    }

    bool do_read(TYPE& data) const {
        const auto write_head = reinterpret_cast<const Head*>(&buffer_[OFFSET_WRITE_HEAD]);
        const auto read_head = reinterpret_cast<Head*>(&buffer_[OFFSET_READ_HEAD]);

        if (read_head->offset == write_head->offset && read_head->lap == write_head->lap) {
            return false;
        }
        switch (write_head->lap - read_head->lap) {
            case 0: // same lap
                break;
            case 1: // one lap ahead
                if (write_head->offset > read_head->offset) {
                    read_head->offset = write_head->offset;
                }
                break;
            default: // more than one lap ahead
                read_head->offset = write_head->offset;
                read_head->lap = write_head->lap - 1;
        }
        ::memcpy(&data, &buffer_[read_head->offset], sizeof(TYPE));
        advance(*read_head);
        return true;
    }

    ~Circular() = default;

private:
    uint8_t* buffer_;
    const uint64_t key_;
    const uint32_t crc_;

    [[nodiscard]] bool is_valid(const Head& head) const {
        if (head.offset < OFFSET_BEGIN_OF_DATA) {
            return false;
        }
        if ((head.offset - OFFSET_BEGIN_OF_DATA) % sizeof(TYPE) != 0) {
            return false;
        }
        return head.offset < OFFSET_END_OF_DATA;
    }

    [[nodiscard]] bool is_valid(const Head& read_head, const Head& write_head) const {
        if (not is_valid(read_head) || not is_valid(write_head)) {
            return false;
        }
        if (read_head.lap > write_head.lap) {
            return false;
        }
        return read_head.lap != write_head.lap || read_head.offset <= write_head.offset;
    }

    [[nodiscard]] bool is_initialized() const {
        const auto read_head = reinterpret_cast<const Head*>(buffer_ + OFFSET_READ_HEAD);
        const auto write_head = reinterpret_cast<const Head*>(buffer_ + OFFSET_WRITE_HEAD);
        const auto key = reinterpret_cast<const uint64_t*>(buffer_ + OFFSET_KEY);
        const auto crc = reinterpret_cast<const uint32_t*>(buffer_ + OFFSET_CRC);
        if (not is_valid(*read_head, *write_head)) {
            return false;
        }

        return *key == key_ && *crc == crc_;
    }

    void initialize() const {
        const auto read_head = reinterpret_cast<Head*>(buffer_ + OFFSET_READ_HEAD);
        const auto write_head = reinterpret_cast<Head*>(buffer_ + OFFSET_WRITE_HEAD);
        const auto key = reinterpret_cast<uint64_t*>(buffer_ + OFFSET_KEY);
        const auto crc = reinterpret_cast<uint32_t*>(buffer_ + OFFSET_CRC);
        constexpr Head zero = { OFFSET_BEGIN_OF_DATA, 0 };
        ::memcpy(read_head, &zero, sizeof(Head));
        ::memcpy(write_head, &zero, sizeof(Head));
        ::memcpy(key, &key_, sizeof(key_));
        ::memcpy(crc, &crc_, sizeof(crc_));
    }

    void advance(Head& head) const {
        head.offset += sizeof(TYPE);
        if (head.offset == OFFSET_END_OF_DATA) {
            head.offset = OFFSET_BEGIN_OF_DATA;
            ++head.lap;
        }
    }
};
}
}
