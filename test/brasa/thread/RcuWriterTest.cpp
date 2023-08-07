#include <brasa/thread/RcuWriter.h>

#include "classes.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

namespace brasa::thread::test {
namespace {
template <typename T>
class PoolMock {
public:
    MOCK_METHOD(void, update, ());
};

} // namespace

TEST(RcuWriterTest, check_update_is_called) {
    PoolMock<int> pool;
    int value = 67;
    EXPECT_CALL(pool, update()).Times(1);
    { // scope for the writer
        const RcuWriter<int, PoolMock<int>> writer(&value, &pool);
        EXPECT_EQ(writer.value(), 67);
    }
}

TEST(RcuWriterTest, check_validity) {
    PoolMock<int> pool;
    int value = 67;
    EXPECT_CALL(pool, update()).Times(1);
    const RcuWriter<int, PoolMock<int>> valid(&value, &pool);
    EXPECT_TRUE(valid.is_valid());
    const RcuWriter<int, PoolMock<int>> invalid1(nullptr, &pool);
    EXPECT_FALSE(invalid1.is_valid());
    const RcuWriter<int, PoolMock<int>> invalid2(&value, nullptr);
    EXPECT_FALSE(invalid2.is_valid());
    const RcuWriter<int, PoolMock<int>> invalid3(nullptr, nullptr);
    EXPECT_FALSE(invalid3.is_valid());
}

TEST(RcuWriterTest, check_after_move_object_is_invalid) {
    PoolMock<int> pool;
    int value = 67;
    EXPECT_CALL(pool, update()).Times(1);
    RcuWriter<int, PoolMock<int>> first(&value, &pool);
    EXPECT_TRUE(first.is_valid());

    auto second = std::move(first);
    EXPECT_TRUE(second.is_valid());
    EXPECT_FALSE(first.is_valid());
}

static_assert(std::is_move_constructible_v<RcuWriter<int, PoolMock<int>>>);
static_assert(std::is_move_constructible_v<RcuWriter<NonMoveable, PoolMock<NonMoveable>>>);
static_assert(false == std::is_copy_constructible_v<RcuWriter<int, PoolMock<int>>>);
static_assert(false == std::is_copy_constructible_v<RcuWriter<NonMoveable, PoolMock<NonMoveable>>>);

} // namespace brasa::thread::test
