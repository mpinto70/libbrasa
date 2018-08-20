#include "brasa/instrument/Singleton.h"

#include <gtest/gtest.h>

namespace brasa {
namespace instrument {

namespace {
struct SemiRegular {
    int x;
    int y;
};

struct Regular {
    int x;
    int y;

    friend bool operator==(const Regular& lhs, const Regular& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

struct TotallyOrdered {
    int x;
    int y;

    friend bool operator==(const TotallyOrdered& lhs, const TotallyOrdered& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend bool operator<(const TotallyOrdered& lhs, const TotallyOrdered& rhs) {
        if (lhs.x < rhs.x) {
            return true;
        }
        if (lhs.x > rhs.x) {
            return false;
        }
        return lhs.y < rhs.y;
    }
};

template <typename T>
Singleton<T> create(const T& t) {
    Singleton<T> a{};
    a.value = t;
    return a;
}
}

TEST(SingletonTest, static_invariants) {
    EXPECT_EQ(sizeof(int), sizeof(Singleton<int>));
    EXPECT_EQ(sizeof(SemiRegular), sizeof(Singleton<SemiRegular>));

    const auto a = create<int>(23);
    const auto sra = create<SemiRegular>({ 23, 42 });
    const auto ra = create<Regular>({ 23, 42 });
    const Singleton<int> b(a);
    const Singleton<SemiRegular> srb(sra);
    const Singleton<Regular> rb(ra);

    EXPECT_NE(&a.value, &b.value);
    EXPECT_NE(&sra.value, &srb.value);
    EXPECT_NE(&ra.value, &rb.value);
}

namespace {
template <typename T>
void verify_creation_assignment(const T& v0, const T& v1) {
    const std::string msg = typeid(T).name();
    Singleton<T> a{T()};
    a.value = v0;
    EXPECT_EQ(memcmp(&a.value, &v0, sizeof(T)), 0) << msg;

    const Singleton<T> b(a);
    EXPECT_EQ(memcmp(&b.value, &a.value, sizeof(T)), 0) << msg;

    a.value = v1;

    Singleton<T> c;
    c = a;
    EXPECT_EQ(memcmp(&c.value, &a.value, sizeof(T)), 0) << msg;
}
}

TEST(SingletonTest, default_creation_assignment) {
    verify_creation_assignment<int>(3, 5);
    verify_creation_assignment<SemiRegular>({ 12, 47 }, { 28, -89 });
    verify_creation_assignment<Regular>({ 12, 47 }, { 28, -89 });
    verify_creation_assignment<TotallyOrdered>({ 12, 47 }, { 28, -89 });
}

namespace {
template <typename T>
void verify_equality(const T& v0, const T& v1) {
    const std::string msg = typeid(T).name();
    EXPECT_FALSE(v0 == v1) << msg;

    Singleton<T> a;
    a.value = v0;

    const Singleton<T> b(a);
    EXPECT_TRUE(a == b) << msg;
    EXPECT_TRUE(a == a) << msg;
    EXPECT_TRUE(b == b) << msg;

    a.value = v1;

    EXPECT_TRUE(a != b) << msg;
}
}

TEST(SingletonTest, equality) {
    verify_equality<int>(3, 5);
    verify_equality<Regular>({ 12, 47 }, { 28, -89 });
    verify_equality<TotallyOrdered>({ 12, 47 }, { 28, -89 });
}

namespace {
template <typename T>
void verify_ordering(const T& v0, const T& v1, const T& v2) {
    const std::string msg = typeid(T).name();
    EXPECT_FALSE(v0 == v1) << msg;
    EXPECT_TRUE(v0 < v1) << msg;
    EXPECT_TRUE(v1 < v2) << msg;

    const auto a = create<T>(v0);
    const auto b = create<T>(v1);
    const auto c = create<T>(v2);
    const auto A = create<T>(v0);
    const auto B = create<T>(v1);
    const auto C = create<T>(v2);

    EXPECT_TRUE(a < b) << msg;
    EXPECT_TRUE(b < c) << msg;
    EXPECT_TRUE(b > a) << msg;
    EXPECT_TRUE(c > b) << msg;
    EXPECT_TRUE(a <= b) << msg;
    EXPECT_TRUE(b <= c) << msg;
    EXPECT_TRUE(b >= a) << msg;
    EXPECT_TRUE(c >= b) << msg;

    EXPECT_TRUE(a <= A) << msg;
    EXPECT_TRUE(a >= A) << msg;
    EXPECT_TRUE(b <= B) << msg;
    EXPECT_TRUE(b >= B) << msg;
    EXPECT_TRUE(c <= C) << msg;
    EXPECT_TRUE(c >= C) << msg;
}
}

TEST(SingletonTest, total_ordering) {
    verify_ordering<int>(3, 5, 7);
    verify_ordering<TotallyOrdered>({ 12, 47 }, { 28, -89 }, { 50, 300 });
}

namespace {
template <typename T>
void verify_conversions(const T& v) {
    const std::string msg = typeid(T).name();

    const Singleton<T> x(v);
    EXPECT_EQ(memcmp(&x.value, &v, sizeof(T)), 0) << msg;

    const T w = static_cast<T>(x);
    EXPECT_EQ(memcmp(&w, &v, sizeof(T)), 0) << msg;
}
}

TEST(SingletonTest, conversions) {
    verify_conversions<int>(3);
    verify_conversions<SemiRegular>({ 12, 47 });
    verify_conversions<Regular>({ 12, 47 });
    verify_conversions<TotallyOrdered>({ 12, 47 });
}
}
}
