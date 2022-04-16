#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa {
namespace buffer {

/** Circular buffer reader.
 * `TYPE_` is the struct that will be stored in the buffer
 * `N_` is the number of objects of type `TYPE_` that can be stored in the buffer
 */
template <typename TYPE_, size_t N_>
class CircularReader : public Circular<TYPE_, N_> {
public:
    using Base = Circular<TYPE_, N_>;

    CircularReader(uint8_t* buffer, uint64_t key) noexcept : Base(buffer, key) {}

    /// Reads `data` from buffer and returns true. If there is no data in buffer, returns false
    bool read(TYPE_& data) const noexcept { return Base::do_read(data); }
};
}
}
