#pragma once

#include <cstddef>

namespace brasa::buffer {

/** Some musings about efficient copying of memory.
 * @param dest destination buffer
 * @param src source buffer
 * @param size number of bytes to copy
 */
void copy_by_byte(void* dest, const void* src, size_t size) noexcept;
void copy_by_qword(void* dest, const void* src, size_t size) noexcept;
void copy_by_qword_dest_aligned(void* dest, const void* src, size_t size) noexcept;

} // namespace brasa::buffer
