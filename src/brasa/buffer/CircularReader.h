#pragma once

#include <brasa/buffer/Circular.h>

namespace brasa {
namespace buffer {

template <typename TYPE_, size_t N_>
class CircularReader : public Circular<TYPE_, N_> {
public:
    using Base = Circular<TYPE_, N_>;

    CircularReader(uint8_t* buffer, uint64_t key)
          : Base(buffer, key) {
    }

    bool read(TYPE_& data) const {
        return Base::do_read(data);
    }
};
}
}
