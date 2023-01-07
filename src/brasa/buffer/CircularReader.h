#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa::buffer {

/** Circular buffer reader.
 * `TYPE_` is the struct that will be stored in the buffer
 * `N_` is the number of objects of type `TYPE_` that can be stored in the buffer
 */
template <typename TYPE_, size_t N_>
class CircularReader : public impl::Circular<TYPE_, N_> {
public:
    using Base = impl::Circular<TYPE_, N_>;
    using Base::BUFFER_SIZE;
    using Base::N;
    using Base::TYPE;

    CircularReader(uint8_t* buffer, uint64_t key) noexcept : Base(buffer, key) {}

    /// Reads `value` from `data` and returns true. If there is no element in `data`, returns false.
    bool read(TYPE_& value) noexcept { return Base::do_read(value); }
};
} // namespace brasa::buffer
