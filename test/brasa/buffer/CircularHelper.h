#include <brasa/buffer/Circular.h>

namespace brasa {
namespace buffer {

struct SimpleData {
    int value;
    char character;
} __attribute__((packed));

inline bool operator==(const SimpleData& x, const SimpleData& y) {
    return x.character == y.character && x.value == y.value;
}
}
}
