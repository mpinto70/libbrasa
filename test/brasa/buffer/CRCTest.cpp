#include "brasa/buffer/CRC.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>

namespace brasa {
namespace buffer {

namespace {


}

TEST(CRCTest, crc) {
    const uint8_t little_endian[] = {
        0xef,
        0xcd,
        0xab,
        0x89,
        0x67,
        0x45,
        0x23,
        0x01,
        0x00
    };
    EXPECT_EQ(crc(0x0123456789abcdef), crc32(little_endian, 8));
}

}
}