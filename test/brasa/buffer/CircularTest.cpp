#include <../test/brasa/buffer/CircularHelper.h>
#include <brasa/buffer/Circular.h>
#include <brasa/buffer/CircularReader.h>
#include <brasa/buffer/CircularWriter.h>

#include <gtest/gtest.h>

namespace brasa {
namespace buffer {

namespace {

using data = SimpleData;

template <typename TYPE, uint32_t N>
void initialize_buffer(uint8_t (&buffer)[Circular<TYPE, N>::BUFFER_SIZE], uint64_t key) {
    ::memset(buffer, 0, sizeof(buffer));
    CircularReader<TYPE, N>(buffer, key);
}

template <typename PARENT>
class CircularMock : public PARENT {
public:
    using PARENT::OFFSET_BEGIN_OF_DATA;
    using PARENT::OFFSET_CRC;
    using PARENT::OFFSET_END_OF_DATA;
    using PARENT::OFFSET_KEY;
    using PARENT::OFFSET_READ_HEAD;
    using PARENT::OFFSET_WRITE_HEAD;

    CircularMock(uint8_t* buffer, const uint64_t key)
          : PARENT(buffer, key) {}
};

template <typename CIRCULAR>
void verify_create_uninitialized(const uint64_t KEY) {
    uint8_t buffer[CIRCULAR::BUFFER_SIZE];
    ::memset(buffer, 0x55, sizeof(buffer));

    const CircularMock<CIRCULAR> circular(buffer, KEY);
    const auto read_head = reinterpret_cast<const Head*>(&buffer[circular.OFFSET_READ_HEAD]);
    EXPECT_EQ(read_head->offset, 0u) << "key = " << KEY;
    EXPECT_EQ(read_head->lap, 0u) << "key = " << KEY;
    const auto write_head = reinterpret_cast<const Head*>(&buffer[circular.OFFSET_WRITE_HEAD]);
    EXPECT_EQ(write_head->offset, 0u) << "key = " << KEY;
    EXPECT_EQ(write_head->lap, 0u) << "key = " << KEY;
    const auto pkey = reinterpret_cast<const uint64_t*>(&buffer[circular.OFFSET_KEY]);
    EXPECT_EQ(*pkey, KEY) << "key = " << KEY;
    const auto pcrc = reinterpret_cast<const uint32_t*>(&buffer[circular.OFFSET_CRC]);
    EXPECT_EQ(*pcrc, crc32(KEY)) << "key = " << KEY;

    uint8_t buffer2[sizeof(buffer)];
    ::memset(buffer2, 0x55, sizeof(buffer2));
    EXPECT_EQ(::memcmp(buffer2, buffer, circular.OFFSET_READ_HEAD), 0) << "key = " << KEY;
    EXPECT_NE(::memcmp(buffer2, buffer, sizeof(buffer)), 0) << "key = " << KEY;
    const CIRCULAR circular2(buffer2, KEY);
    EXPECT_EQ(::memcmp(buffer2, buffer, sizeof(buffer)), 0) << "key = " << KEY;
}
}

TEST(CircularTest, create_uninitialized) {
    verify_create_uninitialized<CircularReader<data, 27>>(12345);
    verify_create_uninitialized<CircularReader<int, 158>>(22222);
    verify_create_uninitialized<CircularWriter<data, 27>>(11111);
    verify_create_uninitialized<CircularWriter<int, 158>>(33333);
}

namespace {
template <typename CIRCULAR>
void verify_create_initialized(const uint64_t KEY) {
    using TYPE = typename CIRCULAR::TYPE;
    using CIRCULAR_CONSTANTS = CircularMock<CIRCULAR>;
    uint8_t buffer[CIRCULAR::BUFFER_SIZE];
    ::memset(buffer, 0x55, sizeof(buffer));

    CIRCULAR circular(buffer, KEY);
    auto read_head = reinterpret_cast<Head*>(&buffer[CIRCULAR_CONSTANTS::OFFSET_READ_HEAD]);
    EXPECT_EQ(read_head->offset, 0u) << "key = " << KEY;
    EXPECT_EQ(read_head->lap, 0u) << "key = " << KEY;
    auto write_head = reinterpret_cast<Head*>(&buffer[CIRCULAR_CONSTANTS::OFFSET_WRITE_HEAD]);
    EXPECT_EQ(write_head->offset, 0u) << "key = " << KEY;
    EXPECT_EQ(write_head->lap, 0u) << "key = " << KEY;

    read_head->offset = sizeof(TYPE);
    write_head->offset = 2 * sizeof(TYPE);
    read_head->lap = 11;
    write_head->lap = 17;

    // if the buffer is already initialized, the creation of a circular buffer does not alter
    // underlying memory
    EXPECT_EQ(read_head->offset, sizeof(TYPE)) << "key = " << KEY;
    EXPECT_EQ(read_head->lap, 11u) << "key = " << KEY;
    EXPECT_EQ(write_head->offset, 2 * sizeof(TYPE)) << "key = " << KEY;
    EXPECT_EQ(write_head->lap, 17u) << "key = " << KEY;

    CIRCULAR circular2(buffer, KEY);

    EXPECT_EQ(read_head->offset, sizeof(TYPE)) << "key = " << KEY;
    EXPECT_EQ(read_head->lap, 11u) << "key = " << KEY;
    EXPECT_EQ(write_head->offset, 2 * sizeof(TYPE)) << "key = " << KEY;
    EXPECT_EQ(write_head->lap, 17u) << "key = " << KEY;
}
}

TEST(CircularTest, create_initialized) {
    verify_create_initialized<CircularReader<data, 47>>(12345);
    verify_create_initialized<CircularReader<int, 357>>(22222);
    verify_create_initialized<CircularWriter<data, 59>>(11111);
    verify_create_initialized<CircularWriter<int, 293>>(33333);
}

TEST(CircularTest, read_fail) {
    using CircularBuffer = CircularReader<data, 15>;
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
    using CircularBuffer = CircularWriter<data, 15>;
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
    ::memcpy(&buffer2[CircularMock<CircularBuffer>::OFFSET_WRITE_HEAD], &write_head, sizeof(Head));

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    d.value += 1;
    d.character += 3;

    circular.write(d);

    EXPECT_NE(::memcmp(buffer, buffer2, sizeof(buffer)), 0);

    write_head.offset += sizeof(data);

    ::memcpy(&buffer2[sizeof(data)], &d, sizeof(d));
    ::memcpy(&buffer2[CircularMock<CircularBuffer>::OFFSET_WRITE_HEAD], &write_head, sizeof(Head));

    EXPECT_EQ(::memcmp(buffer, buffer2, sizeof(buffer)), 0);
}

namespace {
template <typename TYPE, uint32_t N>
void verify_laps(uint64_t key) {
    using CircularBuffer = Circular<TYPE, N>;
    using CIRCULAR_CONSTANTS = CircularMock<CircularBuffer>;
    uint8_t buffer1[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];
    initialize_buffer<TYPE, N>(buffer1, key);
    initialize_buffer<TYPE, N>(buffer2, key);
    const CircularWriter<TYPE, N> writer(buffer1, key);
    const CircularReader<TYPE, N> reader(buffer1, key);

    TYPE t1, t2;
    Head h;
    for (uint32_t i = 0; i < 3 * N; ++i) {
        const uint32_t off_idx = i % N;
        ::memset(&t1, i, sizeof(t1));
        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;
        writer.write(t1);
        EXPECT_NE(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;

        h.offset = ((i + 1) % N) * sizeof(t1);
        h.lap = (i + 1) / N;
        ::memcpy(&buffer2[off_idx * sizeof(t1)], &t1, sizeof(t1));
        ::memcpy(&buffer2[CIRCULAR_CONSTANTS::OFFSET_WRITE_HEAD], &h, sizeof(h));

        EXPECT_EQ(::memcmp(buffer1, buffer2, sizeof(buffer1)), 0) << i << '/' << key;

        EXPECT_TRUE(reader.read(t2)) << i << '/' << key;
        EXPECT_EQ(t1, t2) << i << '/' << key;

        ::memcpy(&buffer2[CIRCULAR_CONSTANTS::OFFSET_READ_HEAD], &h, sizeof(h));

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
    using CircularBuffer = Circular<TYPE, N>;
    uint8_t buffer[CircularBuffer::BUFFER_SIZE];
    initialize_buffer<TYPE, N>(buffer, key);
    const CircularWriter<TYPE, N> writer(buffer, key);
    const CircularReader<TYPE, N> reader(buffer, key);

    constexpr uint32_t MAX = 3 * N + 7;
    TYPE t1;
    for (uint32_t i = 0; i < MAX; ++i) {
        ::memset(&t1, i, sizeof(t1));
        writer.write(t1);
    }

    for (uint32_t i = 0; i < N; ++i) {
        TYPE t2;
        ::memset(&t2, i + MAX - N, sizeof(t2));
        EXPECT_TRUE(reader.read(t1)) << i << '/' << key;
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
