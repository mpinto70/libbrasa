#include <brasa/type_safe/TypeSafe.h>

#include <gtest/gtest.h>

#include <cassert>

namespace brasa {
namespace type_safe {
// ---------- trivial ----------

namespace {
template <typename safe_type>
void check_trivial(
      const safe_type& value,
      const typename safe_type::underlying_type& stored_value) {
    typename safe_type::underlying_type buffer = stored_value;
    // check that safe_type does not add any storage
    auto reinterpreted_value = reinterpret_cast<safe_type*>(&buffer);
    EXPECT_EQ(static_cast<void*>(reinterpreted_value), static_cast<void*>(&buffer));
    EXPECT_EQ(reinterpreted_value->value, buffer);

    // check the value is correct
    EXPECT_EQ(value.value, stored_value);
    EXPECT_EQ(value, safe_type{ stored_value });

    typename safe_type::underlying_type different_value = stored_value + 1;
    EXPECT_NE(value, safe_type{ different_value });
    EXPECT_TRUE(std::is_trivial<safe_type>::value);
    // is packable
    struct test_struct {
        uint8_t value2;
        safe_type value1;
    } __attribute__((packed));

    // there is no additional space allocated for `safe_type`
    EXPECT_EQ(sizeof(test_struct), sizeof(typename safe_type::underlying_type) + 1);

    static_assert(safe_type{ 5 } == safe_type{ 5 });
    static_assert(safe_type{ 4 } != safe_type{ 5 });

    EXPECT_EQ(value, safe_type::to_safe(stored_value));
}
}

TEST(TypeSafeTest, trivial_is_viable) {
    using TRIVIAL = trivial<uint16_t, struct TRIVIAL_>;
    TRIVIAL value{ 18 };
    uint16_t underlying = 18;
    check_trivial(value, underlying);
}

namespace {
template <typename safe_type>
void check_trivial_array(
      const safe_type& value,
      const typename safe_type::underlying_type& stored_value) {
    auto reinterpreted_value = reinterpret_cast<const safe_type*>(&stored_value);
    EXPECT_EQ(
          static_cast<const void*>(reinterpreted_value),
          static_cast<const void*>(&stored_value));

    EXPECT_EQ(sizeof(value), sizeof(stored_value));
    EXPECT_EQ(memcmp(value.value, stored_value, sizeof(stored_value)), 0);
    safe_type equal{};
    std::copy(stored_value, stored_value + sizeof(equal.value), equal.value);
    safe_type different{};
    std::copy(stored_value, stored_value + sizeof(equal.value), equal.value);
    different.value[0] += 1;
    EXPECT_EQ(value, equal);
    EXPECT_NE(value, different);
    struct test_struct {
        uint8_t value2;
        safe_type value1;
    } __attribute__((packed));

    EXPECT_EQ(sizeof(test_struct), sizeof(typename safe_type::underlying_type) + 1);
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
template <typename safe_type>
void check_ordered(
      const safe_type& value,
      const typename safe_type::underlying_type& stored_value) {
    check_trivial<safe_type>(value, stored_value);

    const typename safe_type::underlying_type stored_plus_one = stored_value + 1;
    const typename safe_type::underlying_type stored_minus_one = stored_value - 1;
    EXPECT_LT(value, safe_type{ stored_plus_one });
    EXPECT_GT(value, safe_type{ stored_minus_one });
    EXPECT_LE(value, safe_type{ stored_plus_one });
    EXPECT_LE(value, safe_type{ stored_value });
    EXPECT_GE(value, safe_type{ stored_minus_one });
    EXPECT_GE(value, safe_type{ stored_value });

    static_assert(safe_type{ 5 } > safe_type{ 4 });
    static_assert(safe_type{ 4 } < safe_type{ 5 });
    static_assert(safe_type{ 5 } >= safe_type{ 4 });
    static_assert(safe_type{ 4 } <= safe_type{ 5 });
    static_assert(safe_type{ 5 } >= safe_type{ 5 });
    static_assert(safe_type{ 4 } <= safe_type{ 4 });
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
template <typename safe_type>
void check_scalar(const safe_type& value, const typename safe_type::underlying_type& stored_value) {
    check_ordered<safe_type>(value, stored_value);

    const typename safe_type::underlying_type scalar = 5;
    typename safe_type::underlying_type expected = 5 * value.value;
    EXPECT_EQ(value * scalar, safe_type{ expected });

    safe_type other = value;
    expected = stored_value + 3;
    other += safe_type{ 3 };
    EXPECT_EQ(other, safe_type{ expected });
    EXPECT_EQ(value + safe_type{ 3 }, other);

    other = value;
    expected = stored_value - 3;
    other -= safe_type{ 3 };
    EXPECT_EQ(other, safe_type{ expected });
    EXPECT_EQ(value - safe_type{ 3 }, other);

    typename safe_type::underlying_type factor = 3;
    expected = stored_value * factor;
    other = value;
    other *= factor;
    EXPECT_EQ(other, safe_type{ expected });
    EXPECT_EQ(value * factor, other);
    EXPECT_EQ(factor * value, other);

    factor = 5;
    expected = stored_value / factor;
    other = value;
    other /= factor;
    EXPECT_EQ(other, safe_type{ expected });
    EXPECT_EQ(value / factor, other);

    static_assert(safe_type{ 5 } + safe_type{ 7 } == safe_type{ 12 });
    static_assert(safe_type{ 7 } - safe_type{ 5 } == safe_type{ 2 });
    constexpr typename safe_type::underlying_type FACTOR = 4;
    static_assert(safe_type{ 12 } * FACTOR == safe_type{ 48 });
    static_assert(FACTOR * safe_type{ 15 } == safe_type{ 60 });
    static_assert(safe_type{ 27 } / FACTOR == safe_type{ 6 });

    static_assert(safe_type{ 12 } * int64_t(5) == safe_type{ 60 });
    static_assert(6 * safe_type{ 15 } == safe_type{ 90 });
    static_assert(safe_type{ 27 } / 4 == safe_type{ 6 });
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
