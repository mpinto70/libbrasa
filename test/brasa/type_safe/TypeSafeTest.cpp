#include <brasa/type_safe/TypeSafe.h>

#include <gtest/gtest.h>

#include <cassert>

namespace brasa {
namespace type_safe {
// ---------- trivial ----------

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
}

TEST(TypeSafeTest, trivial_is_viable) {
    using TRIVIAL = trivial<uint16_t, struct TRIVIAL_>;
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
}

TEST(TypeSafeTest, trivial_is_viable_for_arrays) {
    using TRIVIAL = trivial<char[5], struct TRIVIAL_>;
    TRIVIAL value = { { 'A', 'B', 'C', 'D', 'E' } };
    const char underlying[] = { 'A', 'B', 'C', 'D', 'E' };
    check_trivial_array(value, underlying);

    constexpr TRIVIAL value_eq = { { 'A', 'B', 'C', 'D', 'E' } };
    constexpr TRIVIAL value_ne = { { 'a', 'B', 'C', 'D', 'E' } };
    static_assert(value_eq == value_eq);
    static_assert(value_eq != value_ne);
}

// ---------- ordered ----------
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
}

TEST(TypeSafeTest, ordered_is_viable) {
    using ORDERED = ordered<uint16_t, struct ORDERED_>;

    ORDERED value{ 127 };
    uint16_t underlying = 127;

    check_ordered(value, underlying);
}

TEST(TypeSafeTest, ordered_string_is_viable) {
    constexpr size_t N = 5;
    using ORDERED = ordered<char[N], struct ORDERED_>;

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
    EXPECT_GT(str_2, str_1);

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

// ---------- scalar ----------
namespace {
template <typename SAFE_TYPE>
void check_scalar(const SAFE_TYPE& value, const typename SAFE_TYPE::underlying_type& stored_value) {
    check_ordered<SAFE_TYPE>(value, stored_value);

    const typename SAFE_TYPE::underlying_type scalar = 5;
    typename SAFE_TYPE::underlying_type expected = 5 * value.value;
    EXPECT_EQ(value * scalar, SAFE_TYPE{ expected });

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
}

TEST(TypeSafeTest, scalar_is_viable) {
    using SCALAR = scalar<int64_t, struct SCALAR_>;

    SCALAR value{ 127 };
    int64_t underlying = 127;

    check_scalar(value, underlying);
}

TEST(TypeSafeTest, increment_decrement) {
    using SCALAR = scalar<int64_t, struct SCALAR_>;

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
}
}

TEST(TypeSafeUsageTest, type_punning_works) {
    struct Source {
        uint32_t price;
        char name[5];
        uint64_t time;
    } __attribute__((packed));

    using Price = brasa::type_safe::scalar<uint32_t, struct Price_>;
    using Name = brasa::type_safe::trivial<char[5], struct Name_>;
    using Time = brasa::type_safe::ordered<uint64_t, struct Time_>;
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
