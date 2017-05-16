#include "brasa/buffer/Circular.h"

#include <gtest/gtest.h>

namespace brasa {
namespace buffer {

namespace {
template <typename TYPE_, uint32_t N_>
class CircularHelper: public Circular<TYPE_, N_> {
public:
    CircularHelper(uint8_t* buffer, const uint64_t key)
        : Circular<TYPE_, N_>(buffer, key) {
    }
};
struct data {
    int value;
    char character;
} __attribute__((packed));
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

}
}
