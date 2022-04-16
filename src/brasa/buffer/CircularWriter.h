#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa {
namespace buffer {

/** Circular buffer writer.
 * `TYPE_` is the struct that will be stored in the buffer
 * `N_` is the number of objects of type `TYPE_` that can be stored in the buffer
 */
template <typename TYPE_, size_t N_>
class CircularWriter : public Circular<TYPE_, N_> {
public:
    using Base = Circular<TYPE_, N_>;

    CircularWriter(uint8_t* buffer, uint64_t key) noexcept : Circular<TYPE_, N_>(buffer, key) {}

    /// Writes `data` to buffer
    void write(const TYPE_& data) const noexcept { Base::do_write(data); }
};
}
}
