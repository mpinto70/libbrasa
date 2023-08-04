#include <brasa/thread/Rcu.h>

#include <gtest/gtest.h>

namespace brasa::thread {
TEST(RcuTest, creation_with_value) {
    const Rcu<int> rcu(157);
    EXPECT_EQ(rcu.size(), 1u);
}

TEST(RcuTest, get_value) {
    Rcu<int> rcu(157);
    const auto node = rcu.read();
    EXPECT_EQ(node.value(), 157);
}

TEST(RcuTest, write_value) {
    Rcu<int> rcu(157);
    const auto node1 = rcu.read();
    EXPECT_EQ(node1.value(), 157);
    { // scope for writing
        auto node2 = rcu.write();
        EXPECT_EQ(rcu.size(), 2u);
        EXPECT_EQ(node2.value(), 157);
        node2.value() = -98;
        const auto node3 = rcu.read(); // won't see the change
        EXPECT_EQ(node3.value(), 157);
    }
    const auto node4 = rcu.read(); // will see the change
    EXPECT_EQ(node4.value(), -98);
    EXPECT_EQ(rcu.size(), 2u);
}

} // namespace brasa::thread
