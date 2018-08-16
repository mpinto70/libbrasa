#include "brasa/buffer/Circular.h"

namespace brasa {
namespace buffer {

struct SimpleData {
    int value;
    char character;
} __attribute__((packed));

inline bool operator==(const SimpleData& lhs, const SimpleData& rhs) {
    return lhs.character == rhs.character
           && lhs.value == rhs.value;
}
}
}
