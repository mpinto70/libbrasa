#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa {
namespace buffer {

template <typename TYPE_, size_t N_>
class CircularWriter : public Circular<TYPE_, N_> {
public:
    using Base = Circular<TYPE_, N_>;

    CircularWriter(uint8_t* buffer, uint64_t key)
          : Circular<TYPE_, N_>(buffer, key) {
    }

    void write(const TYPE_& data) const {
        Base::do_write(data);
    }
};
}
}
