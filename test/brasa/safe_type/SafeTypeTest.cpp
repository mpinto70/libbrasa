#include <brasa/safe_type/SafeType.h>

#include <gtest/gtest.h>

#include <cassert>
#include <functional>
#include <limits>

namespace brasa {
namespace safe_type {
// ---------- Trivial ----------

namespace {
template <typename SAFE_TYPE>
void check_trivial(
      const SAFE_TYPE& value,
      const typename SAFE_TYPE::underlying_type& stored_value) {
    typename SAFE_TYPE::underlying_type buffer = stored_value;
    // check that SAFE_TYPE does not add any storage
    auto reinterpreted_value = reinterpret_cast<SAFE_TYPE*>(&buffer);
    EXPECT_EQ(static_cast<void*>(reinterpreted_value), static_cast<void*>(&buffer));
    EXPECT_EQ(reinterpreted_value->value, buffer);

    // check the value is correct
    EXPECT_EQ(value.value, stored_value);
    EXPECT_EQ(value, SAFE_TYPE{ stored_value });

    typename SAFE_TYPE::underlying_type different_value = stored_value + 1;
    EXPECT_NE(value, SAFE_TYPE{ different_value });
    EXPECT_TRUE(std::is_trivial<SAFE_TYPE>::value);
    // is packable
    struct test_struct {
        uint8_t value2;
        SAFE_TYPE value1;
    } __attribute__((packed));

    // there is no additional space allocated for `SAFE_TYPE`
    EXPECT_EQ(sizeof(test_struct), sizeof(typename SAFE_TYPE::underlying_type) + 1);

    static_assert(SAFE_TYPE{ 5 } == SAFE_TYPE{ 5 });
    static_assert(SAFE_TYPE{ 4 } != SAFE_TYPE{ 5 });

    EXPECT_EQ(value, SAFE_TYPE::to_safe(stored_value));
}
} // namespace

TEST(SafeTypeTest, trivial_is_viable) {
    using TRIVIAL = Trivial<uint16_t, struct TRIVIAL_>;
    TRIVIAL value{ 18 };
    uint16_t underlying = 18;
    check_trivial(value, underlying);
}

namespace {
template <typename SAFE_TYPE>
void check_trivial_array(
      const SAFE_TYPE& value,
      const typename SAFE_TYPE::underlying_type& stored_value) {
    auto reinterpreted_value = reinterpret_cast<const SAFE_TYPE*>(&stored_value);
    EXPECT_EQ(
          static_cast<const void*>(reinterpreted_value),
          static_cast<const void*>(&stored_value));

    EXPECT_EQ(sizeof(value), sizeof(stored_value));
    EXPECT_EQ(memcmp(value.value, stored_value, sizeof(stored_value)), 0);
    SAFE_TYPE equal{};
    std::copy(stored_value, stored_value + sizeof(equal.value), equal.value);
    SAFE_TYPE different{};
    std::copy(stored_value, stored_value + sizeof(equal.value), equal.value);
    different.value[0] += 1;
    EXPECT_EQ(value, equal);
    EXPECT_NE(value, different);
    struct test_struct {
        uint8_t value2;
        SAFE_TYPE value1;
    } __attribute__((packed));

    EXPECT_EQ(sizeof(test_struct), sizeof(typename SAFE_TYPE::underlying_type) + 1);
}
} // namespace

TEST(SafeTypeTest, trivial_is_viable_for_arrays) {
    using TRIVIAL = Trivial<char[5], struct TRIVIAL_>;
    TRIVIAL value = { { 'A', 'B', 'C', 'D', 'E' } };
    const char underlying[] = { 'A', 'B', 'C', 'D', 'E' };
    check_trivial_array(value, underlying);

    constexpr TRIVIAL value_eq = { { 'A', 'B', 'C', 'D', 'E' } };
    constexpr TRIVIAL value_ne = { { 'a', 'B', 'C', 'D', 'E' } };
    static_assert(value_eq == value_eq);
    static_assert(value_eq != value_ne);
}

// ---------- Ordered ----------
namespace {
template <typename SAFE_TYPE>
void check_ordered(
      const SAFE_TYPE& value,
      const typename SAFE_TYPE::underlying_type& stored_value) {
    check_trivial<SAFE_TYPE>(value, stored_value);

    const typename SAFE_TYPE::underlying_type stored_plus_one = stored_value + 1;
    const typename SAFE_TYPE::underlying_type stored_minus_one = stored_value - 1;
    EXPECT_LT(value, SAFE_TYPE{ stored_plus_one });
    EXPECT_GT(value, SAFE_TYPE{ stored_minus_one });
    EXPECT_LE(value, SAFE_TYPE{ stored_plus_one });
    EXPECT_LE(value, SAFE_TYPE{ stored_value });
    EXPECT_GE(value, SAFE_TYPE{ stored_minus_one });
    EXPECT_GE(value, SAFE_TYPE{ stored_value });

    static_assert(SAFE_TYPE{ 5 } > SAFE_TYPE{ 4 });
    static_assert(SAFE_TYPE{ 4 } < SAFE_TYPE{ 5 });
    static_assert(SAFE_TYPE{ 5 } >= SAFE_TYPE{ 4 });
    static_assert(SAFE_TYPE{ 4 } <= SAFE_TYPE{ 5 });
    static_assert(SAFE_TYPE{ 5 } >= SAFE_TYPE{ 5 });
    static_assert(SAFE_TYPE{ 4 } <= SAFE_TYPE{ 4 });
}
} // namespace

TEST(SafeTypeTest, ordered_is_viable) {
    using ORDERED = Ordered<uint16_t, struct ORDERED_>;

    ORDERED value{ 127 };
    uint16_t underlying = 127;

    check_ordered(value, underlying);
}

TEST(SafeTypeTest, ordered_string_is_viable) {
    constexpr size_t N = 5;
    using ORDERED = Ordered<char[N], struct ORDERED_>;

    constexpr const char cstr_1[N] = "1234";
    constexpr ORDERED str_1{ "1234" };

    constexpr const char cstr_2[N] = "4321";
    constexpr ORDERED str_2{ "4321" };

    static_assert(sizeof(ORDERED) == N);
    static_assert(sizeof(cstr_1) == N);
    static_assert(sizeof(str_1) == N);
    static_assert(sizeof(cstr_2) == N);
    static_assert(sizeof(str_2) == N);

    EXPECT_NE(&str_1.value, &str_2.value);

    EXPECT_STREQ(str_1.value, cstr_1);
    EXPECT_STREQ(str_2.value, cstr_2);
    EXPECT_EQ(str_1, str_1);
    EXPECT_NE(str_1, str_2);
    EXPECT_NE(str_2, str_1);
    EXPECT_LT(str_1, str_2);
    EXPECT_LE(str_1, str_2);
    EXPECT_GT(str_2, str_1);
    EXPECT_GE(str_2, str_1);

    ORDERED str_3 = str_1;
    EXPECT_STREQ(str_3.value, cstr_1);
    EXPECT_STRNE(str_3.value, cstr_2);
    EXPECT_EQ(str_1, str_3);
    EXPECT_NE(str_2, str_3);

    str_3 = str_2;
    EXPECT_STRNE(str_3.value, cstr_1);
    EXPECT_STREQ(str_3.value, cstr_2);
    EXPECT_NE(str_1, str_3);
    EXPECT_EQ(str_2, str_3);
}

// ---------- Scalar ----------
namespace {
template <typename SAFE_TYPE>
void check_scalar(const SAFE_TYPE& value, const typename SAFE_TYPE::underlying_type& stored_value) {
    check_ordered<SAFE_TYPE>(value, stored_value);

    const typename SAFE_TYPE::underlying_type Scalar = 5;
    typename SAFE_TYPE::underlying_type expected = 5 * value.value;
    EXPECT_EQ(value * Scalar, SAFE_TYPE{ expected });

    SAFE_TYPE other = value;
    expected = stored_value + 3;
    other += SAFE_TYPE{ 3 };
    EXPECT_EQ(other, SAFE_TYPE{ expected });
    EXPECT_EQ(value + SAFE_TYPE{ 3 }, other);

    other = value;
    expected = stored_value - 3;
    other -= SAFE_TYPE{ 3 };
    EXPECT_EQ(other, SAFE_TYPE{ expected });
    EXPECT_EQ(value - SAFE_TYPE{ 3 }, other);

    typename SAFE_TYPE::underlying_type factor = 3;
    expected = stored_value * factor;
    other = value;
    other *= factor;
    EXPECT_EQ(other, SAFE_TYPE{ expected });
    EXPECT_EQ(value * factor, other);
    EXPECT_EQ(factor * value, other);

    factor = 5;
    expected = stored_value / factor;
    other = value;
    other /= factor;
    EXPECT_EQ(other, SAFE_TYPE{ expected });
    EXPECT_EQ(value / factor, other);

    static_assert(SAFE_TYPE{ 5 } + SAFE_TYPE{ 7 } == SAFE_TYPE{ 12 });
    static_assert(SAFE_TYPE{ 7 } - SAFE_TYPE{ 5 } == SAFE_TYPE{ 2 });
    constexpr typename SAFE_TYPE::underlying_type FACTOR = 4;
    static_assert(SAFE_TYPE{ 12 } * FACTOR == SAFE_TYPE{ 48 });
    static_assert(FACTOR * SAFE_TYPE{ 15 } == SAFE_TYPE{ 60 });
    static_assert(SAFE_TYPE{ 27 } / FACTOR == SAFE_TYPE{ 6 });

    static_assert(SAFE_TYPE{ 12 } * int64_t(5) == SAFE_TYPE{ 60 });
    static_assert(6 * SAFE_TYPE{ 15 } == SAFE_TYPE{ 90 });
    static_assert(SAFE_TYPE{ 27 } / 4 == SAFE_TYPE{ 6 });
}
} // namespace

TEST(SafeTypeTest, scalar_is_viable) {
    using SCALAR = Scalar<int64_t, struct SCALAR_>;

    SCALAR value{ 127 };
    int64_t underlying = 127;

    check_scalar(value, underlying);
}

TEST(SafeTypeTest, increment_decrement) {
    using SCALAR = Scalar<int64_t, struct SCALAR_>;

    SCALAR value{ 127 };
    const auto pre_increment = ++value;
    const auto pos_increment = value++;
    EXPECT_EQ(value.value, 129);
    EXPECT_EQ(pre_increment.value, 128);
    EXPECT_EQ(pos_increment.value, 128);

    const auto pre_decrement = --value;
    const auto pos_decrement = value--;
    EXPECT_EQ(value.value, 127);
    EXPECT_EQ(pre_decrement.value, 128);
    EXPECT_EQ(pos_decrement.value, 128);
}

namespace {

template <class T>
struct sink {
    typedef void type;
};
template <class T>
using sink_t = typename sink<T>::type;

template <typename T, typename R, typename = void>
struct sum_allowed : std::false_type {};
template <typename T, typename R>
struct sum_allowed<T, R, sink_t<decltype(std::declval<T>() + std::declval<R>())>> : std::true_type {
};
template <typename T, typename R, typename = void>
struct sub_allowed : std::false_type {};
template <typename T, typename R>
struct sub_allowed<T, R, sink_t<decltype(std::declval<T>() - std::declval<R>())>> : std::true_type {
};

template <bool res, typename T, typename R = T>
constexpr void CheckCompilesSumSub() {
    static_assert(sum_allowed<T, R>::value == res);
    static_assert(sub_allowed<T, R>::value == res);
}

template <typename T, typename R, typename = void>
struct mul_allowed : std::false_type {};
template <typename T, typename R>
struct mul_allowed<T, R, sink_t<decltype(std::declval<T>() * std::declval<R>())>> : std::true_type {
};

template <bool res, typename T, typename R = T>
constexpr void CheckCompilesMul() {
    static_assert(mul_allowed<T, R>::value == res);
}

template <typename T, typename R, typename = void>
struct div_allowed : std::false_type {};
template <typename T, typename R>
struct div_allowed<T, R, sink_t<decltype(std::declval<T>() / std::declval<R>())>> : std::true_type {
};

template <bool res, typename T, typename R = T>
constexpr void CheckCompilesDiv() {
    static_assert(div_allowed<T, R>::value == res);
}

template <typename T, typename R, typename = void>
struct less_allowed : std::false_type {};
template <typename T, typename R>
struct less_allowed<T, R, sink_t<decltype(std::declval<T>() < std::declval<R>())>>
      : std::true_type {};
template <typename T, typename R, typename = void>
struct greater_allowed : std::false_type {};
template <typename T, typename R>
struct greater_allowed<T, R, sink_t<decltype(std::declval<T>() > std::declval<R>())>>
      : std::true_type {};
template <typename T, typename R, typename = void>
struct less_equal_allowed : std::false_type {};
template <typename T, typename R>
struct less_equal_allowed<T, R, sink_t<decltype(std::declval<T>() <= std::declval<R>())>>
      : std::true_type {};
template <typename T, typename R, typename = void>
struct greater_equal_allowed : std::false_type {};
template <typename T, typename R>
struct greater_equal_allowed<T, R, sink_t<decltype(std::declval<T>() >= std::declval<R>())>>
      : std::true_type {};

template <bool res, typename T, typename R = T>
constexpr void CheckCompilesOrdering() {
    static_assert(less_allowed<T, R>::value == res);
    static_assert(greater_allowed<T, R>::value == res);
    static_assert(less_equal_allowed<T, R>::value == res);
    static_assert(greater_equal_allowed<T, R>::value == res);
}

template <typename T, typename R, typename = void>
struct eq_allowed : std::false_type {};
template <typename T, typename R>
struct eq_allowed<T, R, sink_t<decltype(std::declval<T>() == std::declval<R>())>> : std::true_type {
};
template <typename T, typename R, typename = void>
struct neq_allowed : std::false_type {};
template <typename T, typename R>
struct neq_allowed<T, R, sink_t<decltype(std::declval<T>() != std::declval<R>())>>
      : std::true_type {};

template <bool res, typename T, typename R = T>
constexpr void CheckCompilesIdentity() {
    static_assert(eq_allowed<T, R>::value == res);
    static_assert(neq_allowed<T, R>::value == res);
}

TEST(SafeTypeTest, Compilation) {
    using TRIVIAL1 = Trivial<int64_t, struct TRIVIAL1_>;
    using TRIVIAL2 = Trivial<TRIVIAL1::underlying_type, struct TRIVIAL2_>;
    using ORDERED1 = Ordered<int64_t, struct ORDERED1_>;
    using ORDERED2 = Ordered<ORDERED1::underlying_type, struct ORDERED2_>;
    using SCALAR1 = Scalar<int64_t, struct SCALAR1_>;
    using SCALAR2 = Scalar<SCALAR1::underlying_type, struct SCALAR2_>;

    // arithmetic operations

    CheckCompilesSumSub<true, int>();
    CheckCompilesSumSub<false, TRIVIAL1>();
    CheckCompilesSumSub<false, ORDERED1>();
    CheckCompilesSumSub<true, SCALAR1>();

    CheckCompilesSumSub<true, int, int>();
    CheckCompilesSumSub<true, int, long>();
    CheckCompilesSumSub<true, long, int>();
    CheckCompilesSumSub<false, SCALAR1, SCALAR1::underlying_type>(); // this is the one to avoid
    CheckCompilesSumSub<false, SCALAR1, SCALAR2>();                  // different types

    // scaling

    CheckCompilesMul<true, int>();
    CheckCompilesMul<false, TRIVIAL1>();
    CheckCompilesMul<false, ORDERED1>();
    CheckCompilesMul<false, SCALAR1>();

    CheckCompilesMul<true, int, long>();
    CheckCompilesMul<false, TRIVIAL1, TRIVIAL1::underlying_type>();
    CheckCompilesMul<false, TRIVIAL1::underlying_type, TRIVIAL1>();
    CheckCompilesMul<false, ORDERED1, ORDERED1::underlying_type>();
    CheckCompilesMul<false, ORDERED1::underlying_type, ORDERED1>();
    CheckCompilesMul<true, SCALAR1, SCALAR1::underlying_type>();
    CheckCompilesMul<true, SCALAR1::underlying_type, SCALAR1>();

    CheckCompilesDiv<true, int>();
    CheckCompilesDiv<false, TRIVIAL1>();
    CheckCompilesDiv<false, ORDERED1>();
    CheckCompilesDiv<false, SCALAR1>();

    CheckCompilesDiv<true, int, long>();
    CheckCompilesDiv<false, TRIVIAL1, TRIVIAL1::underlying_type>();
    CheckCompilesDiv<false, TRIVIAL1::underlying_type, TRIVIAL1>();
    CheckCompilesDiv<false, ORDERED1, ORDERED1::underlying_type>();
    CheckCompilesDiv<false, ORDERED1::underlying_type, ORDERED1>();
    CheckCompilesDiv<true, SCALAR1, SCALAR1::underlying_type>();
    CheckCompilesDiv<false, SCALAR1::underlying_type, SCALAR1>();

    // ordering

    CheckCompilesOrdering<true, int>();
    CheckCompilesOrdering<false, TRIVIAL1>();
    CheckCompilesOrdering<true, ORDERED1>();
    CheckCompilesOrdering<true, SCALAR1>();

    CheckCompilesOrdering<true, int, int>();
    CheckCompilesOrdering<true, int, long>();
    CheckCompilesOrdering<true, long, int>();
    CheckCompilesOrdering<false, ORDERED1, ORDERED1::underlying_type>(); // this is the one to avoid
    CheckCompilesOrdering<false, ORDERED1, ORDERED2>();                  // different types
    CheckCompilesOrdering<false, SCALAR1, SCALAR1::underlying_type>();   // this is the one to avoid
    CheckCompilesOrdering<false, SCALAR1, SCALAR2>();                    // different types

    // identity

    CheckCompilesIdentity<true, int>();
    CheckCompilesIdentity<true, TRIVIAL1>();
    CheckCompilesIdentity<true, ORDERED1>();
    CheckCompilesIdentity<true, SCALAR1>();

    CheckCompilesIdentity<true, int, int>();
    CheckCompilesIdentity<true, int, long>();
    CheckCompilesIdentity<true, long, int>();
    CheckCompilesIdentity<false, TRIVIAL1, TRIVIAL1::underlying_type>(); // this is the one to avoid
    CheckCompilesIdentity<false, TRIVIAL1, TRIVIAL2>();                  // different types
    CheckCompilesIdentity<false, ORDERED1, ORDERED1::underlying_type>(); // this is the one to avoid
    CheckCompilesIdentity<false, ORDERED1, ORDERED2>();                  // different types
    CheckCompilesIdentity<false, SCALAR1, SCALAR1::underlying_type>();   // this is the one to avoid
    CheckCompilesIdentity<false, SCALAR1, SCALAR2>();                    // different types
}
} // namespace

} // namespace safe_type
} // namespace brasa

TEST(SafeTypeUsageTest, type_punning_works) {
    struct Source {
        uint32_t price;
        char name[5];
        uint64_t time;
    } __attribute__((packed));

    using Price = brasa::safe_type::Scalar<uint32_t, struct Price_>;
    using Name = brasa::safe_type::Trivial<char[5], struct Name_>;
    using Time = brasa::safe_type::Ordered<uint64_t, struct Time_>;
    struct Destination {
        Price price;
        Name name;
        Time time;
    } __attribute__((packed));

    static_assert(sizeof(Source) == sizeof(Destination));
    static_assert(offsetof(Source, price) == offsetof(Destination, price));
    static_assert(offsetof(Source, name) == offsetof(Destination, name));
    static_assert(offsetof(Source, time) == offsetof(Destination, time));

    char buffer[sizeof(Source)] = {};

    const Source source = { 0x11223344, { 'A', 'B', 'C', 'D', 'E' }, 0x8877665544332211 };
    memcpy(buffer, &source, sizeof(source));
    Name expected_name = { { 'A', 'B', 'C', 'D', 'E' } };

    auto destination = reinterpret_cast<const Destination*>(buffer);
    EXPECT_EQ(destination->price.value, source.price);
    EXPECT_EQ(memcmp(destination->name.value, source.name, sizeof(source.name)), 0);
    EXPECT_EQ(destination->name, expected_name);
    EXPECT_EQ(destination->time, Time{ 0x8877665544332211 });
    assert(destination->price.value == source.price);
    assert(memcmp(destination->name.value, source.name, sizeof(source.name)) == 0);
    assert(destination->name == expected_name);
    assert(destination->time.value == source.time);
}

namespace {
using QttyProperties = brasa::safe_type::Scalar<uint16_t, struct QttyProperties_>;
constexpr QttyProperties operator"" _qtpp(unsigned long long qtty) {
    return QttyProperties{ static_cast<uint16_t>(qtty) };
}

TEST(SafeTypeUsageTest, suffixes_work) {
    constexpr QttyProperties a{ 12 };
    constexpr auto b = 12_qtpp;
    EXPECT_EQ(a, b);
    static_assert(a == b);
}

} // namespace
