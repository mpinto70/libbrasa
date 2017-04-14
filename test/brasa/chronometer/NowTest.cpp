
#include "brasa/chronometer/NowStd.h"
#include "brasa/chronometer/NowClock.h"

#include <gtest/gtest.h>

#include <ctime>

namespace chronometer {
namespace {
constexpr timespec SLEEP = {0, 50000}; ///< 50 microseconds
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
    for (unsigned i = 0; i < 100; ++i) {
        const uint64_t t0 = NowWallClock();
        EXPECT_EQ(::nanosleep(&SLEEP, nullptr), 0) << "iteration " << i;
        const uint64_t t1 = NowWallClock();
        EXPECT_LT(t0, t1) << "iteration " << i;
    }
}

}
