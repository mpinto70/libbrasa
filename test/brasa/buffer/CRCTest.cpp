#include <brasa/buffer/CRC.h>

#include <gtest/gtest.h>

#include <cstdlib>
#include <format>

namespace brasa::buffer::detail {

/** Standard CRC-32 test vector: "123456789" must produce 0xcbf43926. */
TEST(CRCTest, buffer_known_vector) {
    const uint8_t data[] = "123456789";
    EXPECT_EQ(crc32(data, 9), 0xcbf4'3926u);
}

/** CRC-32 of an empty buffer must be zero (0xFFFFFFFF init ^ 0xFFFFFFFF final = 0). */
TEST(CRCTest, buffer_empty) {
    const uint8_t* data = nullptr;
    EXPECT_EQ(crc32(data, 0), 0x0000'0000u);
}

/** Single-byte CRC-32 known vector: 0x00 → 0xd202ef8d. */
TEST(CRCTest, buffer_single_byte) {
    const uint8_t data[] = { 0x00 };
    EXPECT_EQ(crc32(data, 1), 0xd202'ef8dU);
}

/**
 * The uint64_t overload must produce the same checksum as the buffer overload
 * applied to the 8 raw bytes of the value (in native byte order).
 */
TEST(CRCTest, uint64_consistent_with_buffer) {
    srand(42);
    for (size_t i = 0; i < 1000; ++i) {
        const uint64_t val = (uint64_t(rand()) << 32) | uint32_t(rand());
        SCOPED_TRACE(std::format("iteration {} with value {}", i, val));
        const auto* buffer = reinterpret_cast<const uint8_t*>(&val);
        EXPECT_EQ(crc32(val), crc32(buffer, sizeof(val)));
    }
}

} // namespace brasa::buffer::detail
