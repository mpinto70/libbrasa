#include <brasa/buffer/Copy.h>

#include <algorithm>
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

void copy_by_qword_dest_aligned(void* dest, const void* src, size_t size) noexcept {
    const size_t align_offset = reinterpret_cast<size_t>(dest) & 0x7;
    const size_t pre_copy_size = std::min(align_offset, size);
    copy_by_byte(dest, src, pre_copy_size);
    dest = static_cast<uint8_t*>(dest) + pre_copy_size;
    src = static_cast<const uint8_t*>(src) + pre_copy_size;
    size -= pre_copy_size;
    copy_by_qword(dest, src, size);
}

} // namespace brasa::buffer
