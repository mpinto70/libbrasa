#include "brasa/buffer/Circular.h"

namespace brasa {
namespace buffer {

template <typename TYPE_, uint32_t N_>
class CircularHelper: public Circular<TYPE_, N_> {
public:
    using Base = Circular<TYPE_, N_>;
    CircularHelper(uint8_t* buffer, const uint64_t key)
        : Circular<TYPE_, N_>(buffer, key) {
    }
    void do_write(const TYPE_& data) const {
        Base::write(data);
    }
    bool do_read(TYPE_& data) const {
        return Base::read(data);
    }
};

}
}
