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

void copy_by_qword(void* dest, const void* src, size_t size) noexcept {
    while (size >= 8) {
        *static_cast<uint64_t*>(dest) = *static_cast<const uint64_t*>(src);
        dest = static_cast<uint64_t*>(dest) + 1;
        src = static_cast<const uint64_t*>(src) + 1;
        size -= 8;
    }
    copy_by_byte(dest, src, size);
}

} // namespace brasa::buffer
