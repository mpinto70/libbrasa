#include <brasa/chronus/Constants.h>
#include <brasa/chronus/Now.h>

#include <gtest/gtest.h>

#include <ctime>

namespace brasa {
namespace chronus {
namespace {
template <typename NOW_FUNC>
void verifyNow(NOW_FUNC func) {
    for (unsigned i = 0; i < 100; ++i) {
        const uint64_t t0 = func();
        const uint64_t t1 = func();
        EXPECT_LT(t0, t1) << "iteration " << i;
    }
}

template <typename NOW_FUNC>
void verifyUniformity(NOW_FUNC func) {
    constexpr timespec SLEEP = { 0, 50 * NSECS_PER_MSEC };
    const auto t1 = func();
    EXPECT_EQ(::nanosleep(&SLEEP, nullptr), 0);
    const auto t2 = func();
    uint64_t diff = t2 - t1;
    EXPECT_GT(diff, 50 * NSECS_PER_MSEC);
}
} // namespace

TEST(NowTest, nano_now) {
    verifyNow(nano_now);
    verifyUniformity(nano_now);
}

} // namespace chronus
} // namespace brasa
