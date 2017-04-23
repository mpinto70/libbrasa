#pragma once

#include <chrono>
#include <cstdint>

namespace chronometer {
inline uint64_t NowStd() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
}
