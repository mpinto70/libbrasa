
#include "brasa/chronometer/NowStd.h"
#include "brasa/chronometer/NowClock.h"

#include <gtest/gtest.h>

#include <ctime>

namespace chronometer {
namespace {
template <typename NOW_FUNC>
void verifyNow(NOW_FUNC func) {
    for (unsigned i = 0; i < 100; ++i) {
        const uint64_t t0 = func();
        const uint64_t t1 = func();
        EXPECT_LT(t0, t1) << "iteration " << i;
    }
}
}

TEST(NowTest, nowStd) {
    verifyNow(NowStd);
}

TEST(NowTest, nowClockRealTime) {
    verifyNow(NowClock<CLOCK_REALTIME>());
}

TEST(NowTest, nowClockProcessTime) {
    verifyNow(NowClock<CLOCK_PROCESS_CPUTIME_ID>());
}

}
