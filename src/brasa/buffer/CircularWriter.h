#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa::buffer {

/** Circular buffer writer.
 * `TYPE_` is the struct that will be stored in the buffer
 * `N_` is the number of objects of type `TYPE_` that can be stored in the buffer
 */
template <typename TYPE_, size_t N_>
class CircularWriter : public impl::Circular<TYPE_, N_> {
public:
    using Base = impl::Circular<TYPE_, N_>;
    using Base::BUFFER_SIZE;
    using Base::N;
    using Base::TYPE;

    CircularWriter(uint8_t* buffer, uint64_t key) noexcept : Base(buffer, key) {}

    /// Writes `value` to `data`
    void write(const TYPE_& value) noexcept { Base::do_write(value); }
};
}
