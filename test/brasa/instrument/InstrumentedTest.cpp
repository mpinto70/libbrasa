#include <brasa/instrument/Instrumented.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <vector>

namespace brasa {
namespace instrument {

namespace {
template <typename I, typename N>
void Iota(I first, I last, N start = N(0), N step = N(1)) {
    using T = typename std::iterator_traits<I>::value_type;
    while (first != last) {
        *first = T(start);
        start += step;
        ++first;
    }
}
} // namespace

TEST(InstrumentedTest, names) {
    EXPECT_EQ(
          sizeof(InstrumentedCounter::counter_names),
          InstrumentedCounter::NUMBER_OPS * sizeof(char*));
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::n);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::destruction);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::default_construction);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::conversion_construction);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::conversion_move_construction);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::copy_construction);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::move_construction);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::conversion);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::assignment);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::equality);
    ASSERT_GT(InstrumentedCounter::NUMBER_OPS, InstrumentedCounter::comparison);

    const char** names = InstrumentedCounter::counter_names;
    EXPECT_EQ(std::string(names[InstrumentedCounter::n]), "n");
    EXPECT_EQ(std::string(names[InstrumentedCounter::destruction]), "dtor");
    EXPECT_EQ(std::string(names[InstrumentedCounter::default_construction]), "default ctor");
    EXPECT_EQ(std::string(names[InstrumentedCounter::conversion_construction]), "conv ctor");
    EXPECT_EQ(
          std::string(names[InstrumentedCounter::conversion_move_construction]),
          "conv move ctor");
    EXPECT_EQ(std::string(names[InstrumentedCounter::copy_construction]), "copy ctor");
    EXPECT_EQ(std::string(names[InstrumentedCounter::move_construction]), "move ctor");
    EXPECT_EQ(std::string(names[InstrumentedCounter::conversion]), "conv");
    EXPECT_EQ(std::string(names[InstrumentedCounter::assignment]), "assign");
    EXPECT_EQ(std::string(names[InstrumentedCounter::equality]), "equal");
    EXPECT_EQ(std::string(names[InstrumentedCounter::comparison]), "compare");
}

namespace {
template <typename T, size_t N>
void run_sort() {
    std::vector<Instrumented<int>> vec(N);
    Iota(vec.begin(), vec.end(), 0);
    std::random_shuffle(vec.begin(), vec.end());

    InstrumentedCounter::initialize(N);

    std::sort(vec.begin(), vec.end());

    EXPECT_EQ(N, InstrumentedCounter::counts[InstrumentedCounter::n]) << N;
    EXPECT_NE(0u, InstrumentedCounter::counts[InstrumentedCounter::move_construction]) << N;
    EXPECT_NE(0u, InstrumentedCounter::counts[InstrumentedCounter::destruction]) << N;
    EXPECT_NE(0u, InstrumentedCounter::counts[InstrumentedCounter::assignment]) << N;
    EXPECT_NE(0u, InstrumentedCounter::counts[InstrumentedCounter::comparison]) << N;

    EXPECT_EQ(0u, InstrumentedCounter::counts[InstrumentedCounter::conversion_construction]) << N;
    EXPECT_EQ(0u, InstrumentedCounter::counts[InstrumentedCounter::conversion_move_construction])
          << N;
    EXPECT_EQ(0u, InstrumentedCounter::counts[InstrumentedCounter::conversion]) << N;
    EXPECT_EQ(0u, InstrumentedCounter::counts[InstrumentedCounter::copy_construction]) << N;
    EXPECT_EQ(0u, InstrumentedCounter::counts[InstrumentedCounter::default_construction]) << N;
    EXPECT_EQ(0u, InstrumentedCounter::counts[InstrumentedCounter::equality]) << N;
}
} // namespace

TEST(InstrumentedTest, sort) {
    run_sort<int, 16>();
    run_sort<int, 1000000>();
}

namespace {
template <typename F>
void verify_operations(F f, const std::vector<int>& ops) {
    std::map<int, size_t> operations;
    for (const auto op : ops) {
        operations[op]++;
    }
    InstrumentedCounter::initialize(0);
    f();
    for (int i = 0; i < InstrumentedCounter::NUMBER_OPS; ++i) {
        const std::string msgi = InstrumentedCounter::counter_names[i];
        const auto it = operations.find(i);
        if (it == operations.end()) {
            EXPECT_EQ(0u, InstrumentedCounter::counts[i]) << msgi;
        } else {
            EXPECT_EQ(it->second, InstrumentedCounter::counts[i]) << msgi;
        }
    }
}
} // namespace

TEST(InstrumentedTest, count_default_construction) {
    verify_operations(
          []() { const Instrumented<int> x; },
          {
                InstrumentedCounter::default_construction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_conversion_construction) {
    verify_operations(
          []() {
              int i = 13;
              const Instrumented<int> x(i);
          },
          {
                InstrumentedCounter::conversion_construction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_conversion_move_construction) {
    verify_operations(
          []() { const auto x = Instrumented<int>(129); },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_copy_construction) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              const Instrumented<int> y(x);
          },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::copy_construction,
                InstrumentedCounter::destruction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_move_construction) {
    verify_operations(
          []() {
              Instrumented<int> a(123);
              Instrumented<int> b(std::move(a));
          },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::move_construction,
                InstrumentedCounter::destruction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_conversion) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              static_cast<int>(x);
          },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::conversion,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_assignment) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              Instrumented<int> y;
              y = x;
          },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::default_construction,
                InstrumentedCounter::assignment,
                InstrumentedCounter::destruction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_equality) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              Instrumented<int> y;
              y = x;
              y == x;
              y != x;
          },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::default_construction,
                InstrumentedCounter::assignment,
                InstrumentedCounter::equality,
                InstrumentedCounter::equality,
                InstrumentedCounter::destruction,
                InstrumentedCounter::destruction,
          });
}

TEST(InstrumentedTest, count_comparison) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              Instrumented<int> y;
              y = x;
              y < x;
              y > x;
              y <= x;
              y >= x;
          },
          {
                InstrumentedCounter::conversion_move_construction,
                InstrumentedCounter::default_construction,
                InstrumentedCounter::assignment,
                InstrumentedCounter::comparison,
                InstrumentedCounter::comparison,
                InstrumentedCounter::comparison,
                InstrumentedCounter::comparison,
                InstrumentedCounter::destruction,
                InstrumentedCounter::destruction,
          });
}
} // namespace instrument
} // namespace brasa
