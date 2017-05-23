#include "CircularHelper.h"
#include "brasa/buffer/Circular.h"

#include <gtest/gtest.h>

namespace brasa {
namespace buffer {

namespace {

using data = SimpleData;

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
    EXPECT_FALSE(circular.read(d));

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
    circular.write(d);

    EXPECT_NE(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    Head write_head = { sizeof(data), 0 };
    ::memcpy(buffer2, &d, sizeof(d));
    ::memcpy(&buffer2[CircularBuffer::OFFSET_WRITE_HEAD], &write_head, sizeof(Head));

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    d.value += 1;
    d.character += 3;

    circular.write(d);

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
        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;
        circular.write(t1);
        EXPECT_NE(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;

        h.offset = ((i + 1) % N) * sizeof(t1);
        h.lap = (i + 1) / N;
        ::memcpy(&buffer2[off_idx * sizeof(t1)], &t1, sizeof(t1));
        ::memcpy(&buffer2[CircularBuffer::OFFSET_WRITE_HEAD], &h, sizeof(h));

        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;

        EXPECT_TRUE(circular.read(t2)) << i << '/' << key;
        EXPECT_EQ(t1, t2) << i << '/' << key;

        ::memcpy(&buffer2[CircularBuffer::OFFSET_READ_HEAD], &h, sizeof(h));

        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;
    }
}
}

TEST(CircularTest, laps) {
    verify_laps<data, 29>(12345678);
    verify_laps<int, 17>(87654321);
}

namespace {
template <typename TYPE, uint32_t N>
void verify_many_laps_one_read(uint64_t key) {
    using CircularBuffer = CircularHelper<TYPE, N>;
    uint8_t buffer[CircularBuffer::BUFFER_SIZE];
    initialize_buffer<TYPE, N>(buffer, key);
    const CircularBuffer circular(buffer, key);

    constexpr size_t MAX = 3 * N + 7;
    TYPE t1;
    for (size_t i = 0; i < MAX; ++i) {
        ::memset(&t1, i, sizeof(t1));
        circular.write(t1);
    }

    for (size_t i = 0; i < N; ++i) {
        TYPE t2;
        ::memset(&t2, i + MAX - N, sizeof(t2));
        EXPECT_TRUE(circular.read(t1)) << i << '/' << key;
        EXPECT_EQ(t1, t2) << i << '/' << key;
    }
}
}

TEST(CircularTest, manyLapsOneRead) {
    verify_many_laps_one_read<data, 278>(12);
    verify_many_laps_one_read<data, 27>(13);
    verify_many_laps_one_read<int, 278>(14);
    verify_many_laps_one_read<int, 27>(15);
}

}
}
