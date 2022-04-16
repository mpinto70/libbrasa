#include <brasa/buffer/CRC.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>

namespace brasa {
namespace buffer {

TEST(CRCTest, crc32) {
    srand(time(nullptr));
    for (size_t i = 0; i < 1000; ++i) {
        const uint64_t val = uint64_t(rand()) << 32 + rand();
        auto buffer = reinterpret_cast<const uint8_t*>(&val);
        EXPECT_EQ(crc32(val), crc32(buffer, sizeof(val)));
    }
}
}
}
