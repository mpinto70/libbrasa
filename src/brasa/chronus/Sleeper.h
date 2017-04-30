#pragma once

#include <cstdint>
#include <ctime>

namespace brasa {
namespace chronus {

class NanoSleeper {
public:
    void operator()(const uint32_t sleep_length) const {
        const timespec SLEEP = {0, sleep_length};
        ::nanosleep(&SLEEP, nullptr);
    }
};

}
}
