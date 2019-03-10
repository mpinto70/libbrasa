#include "brasa/type_safe/TypeSafe.h"

#include <gtest/gtest.h>

namespace brasa {
namespace type_safe {
////////// trivial //////////

namespace {
template <typename safe_type>
void check_trivial(const safe_type& value, const typename safe_type::underlying_type stored_value) {
    typename safe_type::underlying_type buffer = stored_value;
    auto reinterpreted_value = reinterpret_cast<safe_type*>(&buffer);
    EXPECT_EQ(static_cast<void*>(reinterpreted_value), static_cast<void*>(&buffer));
    EXPECT_EQ(reinterpreted_value->value, buffer);

    EXPECT_EQ(value.value, stored_value);
    EXPECT_EQ(value, safe_type{ stored_value });
    typename safe_type::underlying_type different_value = stored_value + 1;
    EXPECT_NE(value, safe_type{ different_value });
    EXPECT_TRUE(std::is_trivial<safe_type>::value);
    EXPECT_TRUE(std::is_pod<safe_type>::value);
    // is packable
    struct test_struct {
        safe_type value1;
        uint8_t value2;
    } __attribute__((packed));

    EXPECT_EQ(sizeof(test_struct), sizeof(typename safe_type::underlying_type) + 1);

    static_assert(safe_type{ 5 } == safe_type{ 5 });
    static_assert(safe_type{ 4 } != safe_type{ 5 });
}
}

TEST(TypeSafeTest, trivial_is_viable) {
    using TRIVIAL = trivial<uint16_t, struct TRIVIAL_>;
    TRIVIAL value{ 18 };
    uint16_t underlying = 18;
    check_trivial(value, underlying);
}

////////// ordered //////////
namespace {
template <typename safe_type>
void check_ordered(const safe_type& value, const typename safe_type::underlying_type stored_value) {
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

////////// scalar //////////
namespace {
template <typename safe_type>
void check_scalar(const safe_type& value, const typename safe_type::underlying_type stored_value) {
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
}
}

TEST(TypeSafeTest, scalar_is_viable) {
    using SCALAR = scalar<uint16_t, struct SCALAR_>;

    SCALAR value{ 127 };
    uint16_t underlying = 127;

    check_scalar(value, underlying);
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
    using Time = brasa::type_safe::ordered<uint64_t, struct Name_>;
    struct Destination {
        Price price;
        Name name;
        Time time;
    } __attribute__((packed));

    static_assert(sizeof(Source) == sizeof(Destination));

    char* buffer[sizeof(Source)] = {};

    const Source source = { 0x11223344, { 'A', 'B', 'C', 'D', 'E' }, 0x8877665544332211 };
    memcpy(buffer, &source, sizeof(source));

    auto destination = reinterpret_cast<const Destination*>(buffer);
    EXPECT_EQ(destination->price, Price{ 0x11223344 });
    EXPECT_EQ(memcmp(destination->name.value, "ABCDE", 5), 0);
    EXPECT_EQ(destination->time, Time{ 0x8877665544332211 });
}
