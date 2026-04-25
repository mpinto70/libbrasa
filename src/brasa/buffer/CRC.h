#pragma once

/**
 * CRC-32 checksum utilities for the brasa buffer implementation.
 * @see https://rosettacode.org/wiki/CRC-32
 */

#include <cstddef>
#include <cstdint>

namespace brasa::buffer::detail {

/**
 * Computes the CRC-32 checksum of a byte buffer.
 * @param buf Pointer to the first byte of the input data.
 * @param len Number of bytes to process.
 * @return CRC-32 checksum of the buffer contents.
 */
uint32_t crc32(const uint8_t* buf, size_t len) noexcept;

/**
 * Computes the CRC-32 checksum of a 64-bit integer value.
 * The value is processed byte-by-byte in little-endian order.
 * @param value The 64-bit integer whose checksum is computed.
 * @return CRC-32 checksum of the value.
 */
uint32_t crc32(uint64_t value) noexcept;

} // namespace brasa::buffer::detail
