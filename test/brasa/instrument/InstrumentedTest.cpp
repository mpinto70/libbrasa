#include <brasa/instrument/Instrumented.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <map>
#include <random>
#include <vector>

namespace brasa::instrument {

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
    using Iint = InstrumentedCounter<int>;
    EXPECT_EQ(sizeof(Iint::counter_names), Iint::NUMBER_OPS * sizeof(char*));
    ASSERT_GT(Iint::NUMBER_OPS, Iint::n);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::destruction);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::default_construction);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::conversion_construction);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::conversion_move_construction);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::copy_construction);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::move_construction);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::conversion);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::assignment);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::move_assignment);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::equality);
    ASSERT_GT(Iint::NUMBER_OPS, Iint::comparison);

    EXPECT_EQ(std::string(Iint::counter_names[Iint::n]), "n");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::destruction]), "dtor");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::default_construction]), "default ctor");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::conversion_construction]), "conv ctor");
    EXPECT_EQ(
          std::string(Iint::counter_names[Iint::conversion_move_construction]),
          "conv move ctor");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::copy_construction]), "copy ctor");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::move_construction]), "move ctor");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::conversion]), "conv");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::assignment]), "assign");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::move_assignment]), "move assign");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::equality]), "equal");
    EXPECT_EQ(std::string(Iint::counter_names[Iint::comparison]), "compare");
}

namespace {
template <typename T, size_t N>
void run_sort() {
    std::vector<Instrumented<int>> vec(N);
    Iota(vec.begin(), vec.end(), 0);
    std::mt19937 gen{ std::random_device{}() };
    std::ranges::shuffle(vec, gen);

    InstrumentedCounter<int>::initialize(N);

    std::sort(vec.begin(), vec.end());

    EXPECT_EQ(N, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::n]) << N;
    EXPECT_NE(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::move_construction])
          << N;
    EXPECT_NE(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::destruction]) << N;
    EXPECT_NE(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::move_assignment]) << N;
    EXPECT_NE(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::comparison]) << N;

    EXPECT_EQ(
          0u,
          InstrumentedCounter<int>::counts[InstrumentedCounter<int>::conversion_construction])
          << N;
    EXPECT_EQ(
          0u,
          InstrumentedCounter<int>::counts[InstrumentedCounter<int>::conversion_move_construction])
          << N;
    EXPECT_EQ(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::conversion]) << N;
    EXPECT_EQ(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::copy_construction])
          << N;
    EXPECT_EQ(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::default_construction])
          << N;
    EXPECT_EQ(0u, InstrumentedCounter<int>::counts[InstrumentedCounter<int>::equality]) << N;
}
} // namespace

TEST(InstrumentedTest, sort) {
    run_sort<int, 16>();
    run_sort<int, 1'000'000>();
}

namespace {
template <typename F>
void verify_operations(F f, const std::vector<int>& ops) {
    std::map<int, size_t> operations;
    for (const auto op : ops) {
        operations[op]++;
    }
    InstrumentedCounter<int>::initialize(0);
    f();
    for (int i = 0; i < InstrumentedCounter<int>::NUMBER_OPS; ++i) {
        const std::string msgi = InstrumentedCounter<int>::counter_names[i];
        const auto it = operations.find(i);
        if (it == operations.end()) {
            EXPECT_EQ(0u, InstrumentedCounter<int>::counts[i]) << msgi;
        } else {
            EXPECT_EQ(it->second, InstrumentedCounter<int>::counts[i]) << msgi;
        }
    }
}
} // namespace

TEST(InstrumentedTest, count_default_construction) {
    verify_operations(
          []() { const Instrumented<int> x; },
          {
                InstrumentedCounter<int>::default_construction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_conversion_construction) {
    verify_operations(
          []() {
              int i = 13;
              const Instrumented<int> x(i);
          },
          {
                InstrumentedCounter<int>::conversion_construction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_conversion_move_construction) {
    verify_operations(
          []() { const auto x = Instrumented<int>(129); },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_copy_construction) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              const Instrumented<int> y(x);
          },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::copy_construction,
                InstrumentedCounter<int>::destruction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_move_construction) {
    verify_operations(
          []() {
              Instrumented<int> a(123);
              Instrumented<int> b(std::move(a));
          },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::move_construction,
                InstrumentedCounter<int>::destruction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_conversion) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              (void) static_cast<int>(x);
          },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::conversion,
                InstrumentedCounter<int>::destruction,
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
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::default_construction,
                InstrumentedCounter<int>::assignment,
                InstrumentedCounter<int>::destruction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_move_assignment) {
    verify_operations(
          []() {
              Instrumented<int> x(129);
              Instrumented<int> y;
              y = std::move(x);
          },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::default_construction,
                InstrumentedCounter<int>::move_assignment,
                InstrumentedCounter<int>::destruction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_equality) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              Instrumented<int> y;
              y = x;
              (void) (y == x);
              (void) (y != x);
          },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::default_construction,
                InstrumentedCounter<int>::assignment,
                InstrumentedCounter<int>::equality,
                InstrumentedCounter<int>::equality,
                InstrumentedCounter<int>::destruction,
                InstrumentedCounter<int>::destruction,
          });
}

TEST(InstrumentedTest, count_comparison) {
    verify_operations(
          []() {
              const auto x = Instrumented<int>(129);
              Instrumented<int> y;
              y = x;
              (void) (y < x);
              (void) (y > x);
              (void) (y <= x);
              (void) (y >= x);
          },
          {
                InstrumentedCounter<int>::conversion_move_construction,
                InstrumentedCounter<int>::default_construction,
                InstrumentedCounter<int>::assignment,
                InstrumentedCounter<int>::comparison,
                InstrumentedCounter<int>::comparison,
                InstrumentedCounter<int>::comparison,
                InstrumentedCounter<int>::comparison,
                InstrumentedCounter<int>::destruction,
                InstrumentedCounter<int>::destruction,
          });
}
} // namespace brasa::instrument
