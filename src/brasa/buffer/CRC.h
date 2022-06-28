#pragma once

#include <cstddef>
#include <cstdint>

namespace brasa::buffer::impl {

uint32_t crc32(const uint8_t* buf, size_t len) noexcept;

uint32_t crc32(uint64_t value) noexcept;
}
