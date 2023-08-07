#include <brasa/thread/Rcu.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <future>
#include <set>
#include <source_location>
#include <thread>

namespace brasa::thread::test {
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

namespace {
template <typename T>
std::set<T> reader_func(const Rcu<T>& rcu, const std::set<T>& values, std::source_location loc) {
    SCOPED_TRACE("From line " + std::to_string(loc.line()));
    std::set<T> values_read;
    for (size_t i = 0; i < 100; ++i) {
        const auto node = rcu.read();
        const auto value = node.value();
        EXPECT_NE(values.find(value), values.end()) << "value: " << value;
        values_read.insert(value);
        usleep(10);
    }
    return values_read;
}

template <typename T>
void concurrent_test(
      const std::set<T>& values,
      std::source_location loc = std::source_location::current()) {
    Rcu<T> rcu(*values.begin());
    std::vector<std::thread> threads;
    std::vector<std::future<std::set<T>>> futures;
    for (size_t i = 0; i < 100; ++i) {
        std::packaged_task<std::set<T>(const Rcu<T>&, const std::set<T>&, std::source_location)>
              task(reader_func<T>);
        futures.emplace_back(task.get_future());
        threads.emplace_back(std::move(task), std::ref(rcu), std::ref(values), loc);
    }

    usleep(200);
    for (const auto& value : values) {
        auto writer = rcu.write();
        writer.value() = value;
        usleep(100);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // check that all values were read
    std::set<T> all_values_read;
    for (auto& future : futures) {
        const auto values = future.get();
        all_values_read.insert(values.begin(), values.end());
    }
    EXPECT_EQ(all_values_read, values);
}
} // namespace

TEST(RcuTest, concurrent_access_str) {
    const std::set<std::string> values_str = {
        "mpinto70",                                   //
        "The quick brown fox jump over the lazy dog", //
        "1234567890",                                 //
        "abc",                                        //
        "ABC",                                        //
        "asdf fasdfr gasdfg asdfgdfgsdcxvz lorem ipsum dolor sit amet, consectetur adipiscing "
        "elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.", //
    };

    concurrent_test(values_str);
}

TEST(RcuTest, concurrent_access_int) {
    const std::set<int> values_int = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    concurrent_test(values_int);
}

} // namespace brasa::thread::test
