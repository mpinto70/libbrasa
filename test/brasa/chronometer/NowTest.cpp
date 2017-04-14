
#include "brasa/chronometer/NowStd.h"
#include "brasa/chronometer/NowClock.h"

#include <gtest/gtest.h>

#include <ctime>

namespace chronometer {
namespace {
constexpr timespec SLEEP = {0, 10}; ///< 10 nanoseconds (the system will sleep more)
}

TEST(NowTest, nowStd) {
    for (unsigned i = 0; i < 100; ++i) {
        const uint64_t t0 = NowStd();
        EXPECT_EQ(::nanosleep(&SLEEP, nullptr), 0) << "iteration " << i;
        const uint64_t t1 = NowStd();
        EXPECT_LT(t0, t1) << "iteration " << i;
    }
}

TEST(NowTest, nowClockRealTime) {
    const NowClock<CLOCK_REALTIME> now;
    for (unsigned i = 0; i < 100; ++i) {
        const uint64_t t0 = now();
        EXPECT_EQ(::nanosleep(&SLEEP, nullptr), 0) << "iteration " << i;
        const uint64_t t1 = now();
        EXPECT_LT(t0, t1) << "iteration " << i;
    }
}

}
