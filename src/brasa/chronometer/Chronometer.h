#pragma once

#include <cstdint>

namespace chronometer {

class Chronometer {
public:
    Chronometer(uint32_t id)
        : id_(id) {
    }
    uint32_t id() const { return id_; }
private:
    const uint32_t id_;
};
}
