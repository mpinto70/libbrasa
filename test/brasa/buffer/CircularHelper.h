#include <brasa/buffer/Circular.h>

#include <ostream>

namespace brasa::buffer {

struct SimpleData {
    int value;
    char character;
};

constexpr bool operator==(const SimpleData& x, const SimpleData& y) {
    return x.character == y.character && x.value == y.value;
}

constexpr bool operator!=(const SimpleData& x, const SimpleData& y) {
    return not(x == y);
}

inline std::ostream& operator<<(std::ostream& out, const SimpleData& x) {
    out << "val = " << x.value << " | char = " << x.character;
    return out;
}

namespace impl {
constexpr bool operator==(const Head& x, const Head& y) {
    return x.index == y.index && x.lap == y.lap;
}

constexpr bool operator!=(const Head& x, const Head& y) {
    return not(x == y);
}

template <typename TYPE, uint32_t N>
bool operator==(const BufferData<TYPE, N>& X, const BufferData<TYPE, N>& Y) {
    auto are_equal = [&](const TYPE(&x)[N], const TYPE(&y)[N]) {
        for (size_t i = 0; i < N; ++i) {
            if (x[i] != y[i]) {
                return false;
            }
        }
        return true;
    };
    return X.write_head == Y.write_head && X.read_head == Y.read_head && X.key == Y.key
           && X.crc == Y.crc && are_equal(X.data, Y.data);
}

template <typename TYPE, uint32_t N>
bool operator!=(const BufferData<TYPE, N>& X, const BufferData<TYPE, N>& Y) {
    return not(X == Y);
}

inline std::ostream& operator<<(std::ostream& out, const Head& x) {
    out << "off = " << x.index << " | lap = " << x.lap;
    return out;
}

template <typename TYPE, uint32_t N>
inline std::ostream& operator<<(std::ostream& out, const BufferData<TYPE, N>& x) {
    out << "key = " << x.key << " | crc " << x.crc << "\n";
    out << "WH = [" << x.write_head << "]\n";
    out << "RH = [" << x.read_head << "]\n";

    for (size_t i = 0; i < N; ++i) {
        out << "el " << i << " [" << x.data[i] << "]\n";
    }
    return out;
}
} // namespace impl
} // namespace brasa::buffer
