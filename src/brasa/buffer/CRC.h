#pragma once
#include <cstdint>
#include <cstddef>

namespace brasa {
namespace buffer {

uint32_t crc32(const uint8_t* buf, size_t len);

uint32_t crc(uint64_t value);

}
}