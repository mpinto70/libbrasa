#include <brasa/thread/RcuReader.h>

#include "classes.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

namespace brasa::thread::test {
namespace {
template <typename T>
class PoolMock {
public:
    MOCK_METHOD(void, release, (const T*), (const));
};
} // namespace

TEST(RcuReaderTest, check_relase_is_called) {
    const PoolMock<int> pool;
    const int value = 67;
    EXPECT_CALL(pool, release(&value)).Times(1);
    { // scope for the reader
        const RcuReader<int, PoolMock<int>> reader(&value, &pool);
        EXPECT_EQ(reader.value(), 67);
    }
}

TEST(RcuReaderTest, check_validity) {
    const PoolMock<int> pool;
    const int value = 67;
    EXPECT_CALL(pool, release(&value)).Times(1);
    const RcuReader<int, PoolMock<int>> valid(&value, &pool);
    EXPECT_TRUE(valid.is_valid());
    const RcuReader<int, PoolMock<int>> invalid1(nullptr, &pool);
    EXPECT_FALSE(invalid1.is_valid());
    const RcuReader<int, PoolMock<int>> invalid2(&value, nullptr);
    EXPECT_FALSE(invalid2.is_valid());
    const RcuReader<int, PoolMock<int>> invalid3(nullptr, nullptr);
    EXPECT_FALSE(invalid3.is_valid());
}

TEST(RcuReaderTest, check_after_move_object_is_invalid) {
    const PoolMock<int> pool;
    const int value = 67;
    EXPECT_CALL(pool, release(&value)).Times(1);
    RcuReader<int, PoolMock<int>> first(&value, &pool);
    EXPECT_TRUE(first.is_valid());

    auto second = std::move(first);
    EXPECT_TRUE(second.is_valid());
    EXPECT_FALSE(first.is_valid());
}

static_assert(std::is_move_constructible_v<RcuReader<int, PoolMock<int>>>);
static_assert(std::is_move_constructible_v<RcuReader<NonMoveable, PoolMock<NonMoveable>>>);
static_assert(false == std::is_copy_constructible_v<RcuReader<int, PoolMock<int>>>);
static_assert(false == std::is_copy_constructible_v<RcuReader<NonMoveable, PoolMock<NonMoveable>>>);

} // namespace brasa::thread::test
