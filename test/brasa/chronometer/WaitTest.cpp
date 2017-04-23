
#include "brasa/chronometer/NowStd.h"
#include "brasa/chronometer/Wait.h"

#include <gtest/gtest.h>

#include <ctime>

namespace chronometer {
namespace {
template <typename NOW_FUNC>
void verifyWait(NOW_FUNC func) {
    const auto waiter = make_waiter<NOW_FUNC>(std::move(func));
    for (unsigned i = 0; i < 100; ++i) {
        const auto t0 = NowStd();
        waiter.wait(1 * NSECS_PER_MSEC);
        const auto t1 = NowStd();
        uint64_t diff = t1 - t0;
        EXPECT_GT(diff, 1 * NSECS_PER_MSEC) << "iteration " << i;
        EXPECT_LT(diff, 10 * NSECS_PER_MSEC) << "iteration " << i;
    }
}
}

TEST(WaitTest, wait) {
    verifyWait(NowStd);
}

}
