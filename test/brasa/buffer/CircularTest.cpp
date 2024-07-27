#include <../test/brasa/buffer/CircularHelper.h>

#include <brasa/buffer/Circular.h>
#include <brasa/buffer/CircularReader.h>
#include <brasa/buffer/CircularWriter.h>

#include <gtest/gtest.h>

namespace brasa::buffer::impl {

namespace {

using data = SimpleData;

template <typename TYPE, uint32_t N>
void initialize_buffer(uint8_t (&buffer)[Circular<TYPE, N>::BUFFER_SIZE], uint64_t key) {
    static_assert(Circular<TYPE, N>::BUFFER_SIZE == sizeof(BufferData<TYPE, N>));
    ::memset(buffer, 0, sizeof(buffer));
    CircularReader<TYPE, N>(buffer, key);
}

template <typename PARENT>
class CircularMock : public PARENT {
public:
    using PARENT::BufferDataT;

    CircularMock(uint8_t* buffer, const uint64_t key) : PARENT(buffer, key) {}
};

template <typename TYPE, uint32_t N>
void compare(const TYPE (&x)[N], const TYPE (&y)[N]) {
    for (size_t i = 0; i < N; ++i) {
        EXPECT_EQ(x[i], y[i]) << "for element " << i;
    }
}

template <typename CIRCULAR>
void verify_create_uninitialized(const uint64_t KEY) {
    SCOPED_TRACE("key = " + std::to_string(KEY) + " for " + std::string(typeid(CIRCULAR).name()));

    uint8_t buffer[CIRCULAR::BUFFER_SIZE];
    ::memset(buffer, 0x55, sizeof(buffer));

    using BufferDataT = typename CircularMock<CIRCULAR>::BufferDataT;
    auto buffer_data = reinterpret_cast<BufferDataT*>(buffer);

    CircularMock<CIRCULAR> circular(buffer, KEY);
    EXPECT_EQ(buffer_data->read_head.index, 0u);
    EXPECT_EQ(buffer_data->read_head.lap, 0u);
    EXPECT_EQ(buffer_data->write_head.index, 0u);
    EXPECT_EQ(buffer_data->write_head.lap, 0u);
    EXPECT_EQ(buffer_data->key, KEY);
    EXPECT_EQ(buffer_data->crc, crc32(KEY));

    uint8_t buffer2[CIRCULAR::BUFFER_SIZE];
    ::memset(buffer2, 0x55, sizeof(buffer2));
    auto buffer_data2 = reinterpret_cast<BufferDataT*>(buffer2);
    compare(buffer_data->data, buffer_data2->data);
    EXPECT_NE(*buffer_data2, *buffer_data);
    const CIRCULAR circular2(buffer2, KEY);
    EXPECT_EQ(*buffer_data2, *buffer_data);
}
} // namespace

TEST(CircularTest, create_uninitialized) {
    verify_create_uninitialized<CircularReader<data, 27>>(12'345);
    verify_create_uninitialized<CircularReader<int, 158>>(22'222);
    verify_create_uninitialized<CircularWriter<data, 27>>(11'111);
    verify_create_uninitialized<CircularWriter<int, 158>>(33'333);
}

namespace {
template <typename CIRCULAR>
void verify_create_initialized(const uint64_t KEY) {
    using BufferDataT = typename CircularMock<CIRCULAR>::BufferDataT;

    SCOPED_TRACE("key = " + std::to_string(KEY));

    uint8_t buffer[CIRCULAR::BUFFER_SIZE];
    ::memset(buffer, 0x55, sizeof(buffer));

    CIRCULAR circular(buffer, KEY);
    auto buffer_data = reinterpret_cast<BufferDataT*>(buffer);
    EXPECT_EQ(buffer_data->read_head.index, 0u);
    EXPECT_EQ(buffer_data->read_head.lap, 0u);
    EXPECT_EQ(buffer_data->write_head.index, 0u);
    EXPECT_EQ(buffer_data->write_head.lap, 0u);

    buffer_data->read_head.index = 1;
    buffer_data->write_head.index = 2;
    buffer_data->read_head.lap = 11;
    buffer_data->write_head.lap = 17;

    // if the buffer is already initialized, the creation of a circular buffer does not alter
    // underlying memory
    EXPECT_EQ(buffer_data->read_head.index, 1);
    EXPECT_EQ(buffer_data->read_head.lap, 11u);
    EXPECT_EQ(buffer_data->write_head.index, 2);
    EXPECT_EQ(buffer_data->write_head.lap, 17u);

    CIRCULAR circular2(buffer, KEY);

    EXPECT_EQ(buffer_data->read_head.index, 1);
    EXPECT_EQ(buffer_data->read_head.lap, 11u);
    EXPECT_EQ(buffer_data->write_head.index, 2);
    EXPECT_EQ(buffer_data->write_head.lap, 17u);
}
} // namespace

TEST(CircularTest, create_initialized) {
    verify_create_initialized<CircularReader<data, 47>>(12'345);
    verify_create_initialized<CircularReader<int, 357>>(22'222);
    verify_create_initialized<CircularWriter<data, 59>>(11'111);
    verify_create_initialized<CircularWriter<int, 293>>(33'333);
}

TEST(CircularTest, read_fail) {
    using CircularBuffer = CircularReader<data, 15>;
    using BufferDataT = BufferData<data, 15>;

    uint8_t buffer[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];
    const uint64_t KEY = 0x1234'5678'90ab'cdefUL;
    initialize_buffer<data, 15>(buffer, KEY);
    initialize_buffer<data, 15>(buffer2, KEY);
    auto buffer_data = reinterpret_cast<BufferDataT*>(buffer);
    auto buffer_data2 = reinterpret_cast<BufferDataT*>(buffer2);

    CircularBuffer circular(buffer, KEY);

    EXPECT_EQ(*buffer_data, *buffer_data2);

    data d;
    EXPECT_FALSE(circular.read(d));

    EXPECT_EQ(*buffer_data, *buffer_data2);
}

TEST(CircularTest, write) {
    using CircularBuffer = CircularWriter<data, 15>;
    using BufferDataT = typename CircularMock<CircularBuffer>::BufferDataT;

    uint8_t buffer[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];

    constexpr uint64_t KEY = 0x1234'5678'90ab'cdefUL;

    initialize_buffer<data, 15>(buffer, KEY);
    initialize_buffer<data, 15>(buffer2, KEY);
    auto buffer_data = reinterpret_cast<BufferDataT*>(buffer);
    auto buffer_data2 = reinterpret_cast<BufferDataT*>(buffer2);

    CircularBuffer circular(buffer, KEY);

    EXPECT_EQ(*buffer_data, *buffer_data2);

    data d = { 1234, 'a' };
    circular.write(d);

    EXPECT_NE(*buffer_data, *buffer_data2);

    Head write_head = { 1, 0 };
    ::memcpy(&buffer_data2->data[0], &d, sizeof(d));
    ::memcpy(&buffer_data2->write_head, &write_head, sizeof(Head));

    EXPECT_EQ(*buffer_data, *buffer_data2);

    d.value += 1;
    d.character += 3;

    circular.write(d);

    EXPECT_NE(*buffer_data, *buffer_data2);

    write_head.index += 1;

    ::memcpy(&buffer_data2->data[1], &d, sizeof(d));
    ::memcpy(&buffer_data2->write_head, &write_head, sizeof(Head));

    EXPECT_EQ(*buffer_data, *buffer_data2);
}

namespace {
template <typename TYPE, uint32_t N>
void verify_laps(const uint64_t key) {
    using CircularBuffer = Circular<TYPE, N>;
    using BufferDataT = typename CircularMock<CircularBuffer>::BufferDataT;

    SCOPED_TRACE("For key " + std::to_string(key) + " / N = " + std::to_string(N));

    uint8_t buffer1[CircularBuffer::BUFFER_SIZE];
    uint8_t buffer2[CircularBuffer::BUFFER_SIZE];

    auto buffer_data1 = reinterpret_cast<BufferDataT*>(buffer1);
    auto buffer_data2 = reinterpret_cast<BufferDataT*>(buffer2);

    initialize_buffer<TYPE, N>(buffer1, key);
    initialize_buffer<TYPE, N>(buffer2, key);

    CircularWriter<TYPE, N> writer(buffer1, key);
    CircularReader<TYPE, N> reader(buffer1, key);

    TYPE t1{}, t2{};
    Head h;
    for (uint32_t i = 0; i < 3 * N; ++i) {
        const uint32_t off_idx = i % N;
        SCOPED_TRACE("Offset " + std::to_string(off_idx) + " of index " + std::to_string(i));
        ::memset(&t1, i, sizeof(t1));
        EXPECT_EQ(*buffer_data1, *buffer_data2);
        writer.write(t1);
        EXPECT_NE(*buffer_data1, *buffer_data2);

        h.index = ((i + 1) % N);
        h.lap = (i + 1) / N;
        ::memcpy(&buffer_data2->data[off_idx], &t1, sizeof(t1));
        ::memcpy(&buffer_data2->write_head, &h, sizeof(h));

        EXPECT_EQ(*buffer_data1, *buffer_data2);

        EXPECT_TRUE(reader.read(t2));
        EXPECT_EQ(t1, t2);

        ::memcpy(&buffer_data2->read_head, &h, sizeof(h));

        EXPECT_EQ(*buffer_data1, *buffer_data2);
    }
    EXPECT_EQ(h.lap, 3);
}
} // namespace

TEST(CircularTest, laps) {
    verify_laps<data, 29>(12'345'678);
    verify_laps<int, 17>(87'654'321);
}

namespace {
template <typename TYPE, uint32_t N>
void verify_many_laps_one_read(uint64_t key) {
    using CircularBuffer = Circular<TYPE, N>;

    SCOPED_TRACE("For key " + std::to_string(key));

    uint8_t buffer[CircularBuffer::BUFFER_SIZE];

    initialize_buffer<TYPE, N>(buffer, key);

    CircularWriter<TYPE, N> writer(buffer, key);
    CircularReader<TYPE, N> reader(buffer, key);

    constexpr uint32_t MAX = 3 * N + 7;
    TYPE t1;
    for (uint32_t i = 0; i < MAX; ++i) {
        ::memset(&t1, i, sizeof(t1));
        writer.write(t1);
    }

    for (uint32_t i = 0; i < N; ++i) {
        TYPE t2;
        ::memset(&t2, i + MAX - N, sizeof(t2));
        EXPECT_TRUE(reader.read(t1)) << i;
        EXPECT_EQ(t1, t2) << i;
    }
}
} // namespace

TEST(CircularTest, manyLapsOneRead) {
    verify_many_laps_one_read<data, 278>(12);
    verify_many_laps_one_read<data, 27>(13);
    verify_many_laps_one_read<int, 278>(14);
    verify_many_laps_one_read<int, 27>(15);
}
} // namespace brasa::buffer::impl
