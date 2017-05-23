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
    void write(const TYPE_& data) const {
        Base::do_write(data);
    }
    bool read(TYPE_& data) const {
        return Base::do_read(data);
    }
};

struct SimpleData {
    int value;
    char character;
} __attribute__((packed));

inline bool operator == (const SimpleData& lhs, const SimpleData& rhs) {
    return lhs.character == rhs.character
           && lhs.value == rhs.value;
}

}
}
