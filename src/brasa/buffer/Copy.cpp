#include <brasa/buffer/Copy.h>

#include <cstdint>

namespace brasa::buffer {

void copy_by_byte(void* dest, const void* src, size_t size) noexcept {
    auto d = static_cast<uint8_t*>(dest);
    auto s = static_cast<const uint8_t*>(src);
    while (size-- > 0) {
        *d++ = *s++;
    }
}

} // namespace brasa::buffer
