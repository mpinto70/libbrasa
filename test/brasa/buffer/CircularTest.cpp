#include "brasa/buffer/Circular.h"

#include <gtest/gtest.h>

namespace brasa {
namespace buffer {

namespace {
template <typename TYPE_, uint32_t N_>
class CircularHelper: public Circular<TYPE_, N_> {
public:
    using Base = Circular<TYPE_, N_>;
    CircularHelper(uint8_t* buffer, const uint64_t key)
        : Circular<TYPE_, N_>(buffer, key) {
    }
    void do_write(const TYPE_& data) const {
        Base::write(data);
    }
    bool do_read(TYPE_& data) const {
        return Base::read(data);
    }
};

struct data {
    int value;
    char character;
} __attribute__((packed));

template <typename TYPE, uint32_t N>
void initialize_buffer(uint8_t (&buffer)[Circular<TYPE, N>::BUFFER_SIZE], uint64_t key) {
    ::memset(buffer, 0, sizeof(buffer));
    CircularHelper<TYPE, N>(buffer, key);
}
}

TEST(CircularTest, create_uninitialized) {
    uint8_t buffer[Circular<data, 150>::BUFFER_SIZE];
    ::memset(buffer, 0x55, sizeof(buffer));
    const uint64_t KEY = 0x1234567890abcdefUL;

    const CircularHelper<data, 150> circular(buffer, KEY);
    const auto read_head = reinterpret_cast<const Head*>(&buffer[circular.OFFSET_READ_HEAD]);
    EXPECT_EQ(read_head->offset, 0u);
    EXPECT_EQ(read_head->lap, 0u);
    const auto write_head = reinterpret_cast<const Head*>(&buffer[circular.OFFSET_WRITE_HEAD]);
    EXPECT_EQ(write_head->offset, 0u);
    EXPECT_EQ(write_head->lap, 0u);
    const auto pkey = reinterpret_cast<const uint64_t*>(&buffer[circular.OFFSET_KEY]);
    EXPECT_EQ(*pkey, KEY);
    const auto pcrc = reinterpret_cast<const uint32_t*>(&buffer[circular.OFFSET_CRC]);
    EXPECT_EQ(*pcrc, crc(KEY));

    uint8_t buffer2[sizeof(buffer)];
    ::memset(buffer2, 0x55, sizeof(buffer2));
    EXPECT_EQ(::memcmp(buffer2, buffer, circular.OFFSET_READ_HEAD), 0);
    EXPECT_NE(::memcmp(buffer2, buffer, sizeof(buffer)), 0);
    const CircularHelper<data, 150> circular2(buffer2, KEY);
    EXPECT_EQ(::memcmp(buffer2, buffer, sizeof(buffer)), 0);
}

TEST(CircularTest, create_initialized) {
    uint8_t buffer[Circular<data, 150>::BUFFER_SIZE];
    ::memset(buffer, 0x55, sizeof(buffer));
    const uint64_t KEY = 0xfedcba0987654321UL;

    CircularHelper<data, 150> circular(buffer, KEY);
    auto read_head = reinterpret_cast<Head*>(&buffer[circular.OFFSET_READ_HEAD]);
    EXPECT_EQ(read_head->offset, 0u);
    EXPECT_EQ(read_head->lap, 0u);
    auto write_head = reinterpret_cast<Head*>(&buffer[circular.OFFSET_WRITE_HEAD]);
    EXPECT_EQ(write_head->offset, 0u);
    EXPECT_EQ(write_head->lap, 0u);

    read_head->offset = sizeof(data);
    write_head->offset = 2 * sizeof(data);
    read_head->lap = 11;
    write_head->lap = 17;

    // if the buffer is already initialized, the creation of a circular buffer does not alter
    // underlying memory
    EXPECT_EQ(read_head->offset, sizeof(data));
    EXPECT_EQ(read_head->lap, 11u);
    EXPECT_EQ(write_head->offset, 2 * sizeof(data));
    EXPECT_EQ(write_head->lap, 17u);

    CircularHelper<data, 150> circular2(buffer, KEY);

    EXPECT_EQ(read_head->offset, sizeof(data));
    EXPECT_EQ(read_head->lap, 11u);
    EXPECT_EQ(write_head->offset, 2 * sizeof(data));
    EXPECT_EQ(write_head->lap, 17u);
}

TEST(CircularTest, read_fail) {
    using CircularBuffer = CircularHelper<data, 15>;
    uint8_t buffer[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];
    const uint64_t KEY = 0x1234567890abcdefUL;
    initialize_buffer<data, 15>(buffer, KEY);
    initialize_buffer<data, 15>(buffer2, KEY);

    const CircularBuffer circular(buffer, KEY);

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    data d;
    EXPECT_FALSE(circular.do_read(d));

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);
}

TEST(CircularTest, write) {
    using CircularBuffer = CircularHelper<data, 15>;
    uint8_t buffer[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];
    const uint64_t KEY = 0x1234567890abcdefUL;
    initialize_buffer<data, 15>(buffer, KEY);
    initialize_buffer<data, 15>(buffer2, KEY);

    const CircularBuffer circular(buffer, KEY);

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    data d = { 1234, 'a' };
    circular.do_write(d);

    EXPECT_NE(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    Head write_head = { sizeof(data), 0 };
    ::memcpy(buffer2, &d, sizeof(d));
    ::memcpy(&buffer2[CircularBuffer::OFFSET_WRITE_HEAD], &write_head, sizeof(Head));

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    d.value += 1;
    d.character += 3;

    circular.do_write(d);

    EXPECT_NE(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    write_head.offset += sizeof(data);

    ::memcpy(&buffer2[sizeof(data)], &d, sizeof(d));
    ::memcpy(&buffer2[CircularBuffer::OFFSET_WRITE_HEAD], &write_head, sizeof(Head));

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);
}

namespace {
template <typename TYPE, uint32_t N>
void verify_laps(uint64_t key) {
    using CircularBuffer = CircularHelper<TYPE, N>;
    uint8_t buffer1[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];
    initialize_buffer<TYPE, N>(buffer1, key);
    initialize_buffer<TYPE, N>(buffer2, key);
    const CircularBuffer circular(buffer1, key);

    TYPE t1, t2;
    Head h;
    for (size_t i = 0; i < 3 * N; ++i) {
        const size_t off_idx = i % N;
        ::memset(&t1, i, sizeof(t1));
        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i;
        circular.do_write(t1);
        EXPECT_NE(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i;

        h.offset = ((i + 1) % N) * sizeof(t1);
        h.lap = (i + 1) / N;
        ::memcpy(&buffer2[off_idx * sizeof(t1)], &t1, sizeof(t1));
        ::memcpy(&buffer2[CircularBuffer::OFFSET_WRITE_HEAD], &h, sizeof(h));

        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i;

        EXPECT_TRUE(circular.do_read(t2));
        EXPECT_EQ(::memcmp(&t1, &t2, sizeof(t1)), 0);

        ::memcpy(&buffer2[CircularBuffer::OFFSET_READ_HEAD], &h, sizeof(h));

        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i;
    }
}
}

TEST(CircularTest, laps) {
    verify_laps<data, 29>(12345678);
    verify_laps<int, 17>(12345678);
}

}
}
