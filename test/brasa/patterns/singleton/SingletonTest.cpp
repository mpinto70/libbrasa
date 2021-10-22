#include <brasa/patterns/singleton/Singleton.h>

#include <gtest/gtest.h>

#include <array>

namespace brasa::pattern::test {
namespace {

template <size_t N>
struct Object {
    template <typename... Ts,
          std::enable_if_t<sizeof...(Ts) == N, bool> = true>
    explicit Object(Ts... vals)
          : values{ vals... } {}

    friend bool operator==(const Object& lhs, const Object& rhs) {
        return lhs.values == rhs.values;
    }

    std::array<int, N> values;
};

struct POD {
    char c;
    int i;
    double d;
};

bool operator==(const POD& lhs, const POD& rhs) {
    return lhs.c == rhs.c && lhs.i == rhs.i && lhs.d == rhs.d;
}

static void free_insts() {
    Singleton<int>::free_instance();
    Singleton<double>::free_instance();
    Singleton<char>::free_instance();
    Singleton<Object<1>>::free_instance();
    Singleton<Object<2>>::free_instance();
    Singleton<Object<3>>::free_instance();
    Singleton<Object<4>>::free_instance();
    Singleton<POD>::free_instance();
}

template <typename T, typename... ARGS>
static void test_create(ARGS&&... args) {
    EXPECT_FALSE(Singleton<T>::has_instance());
    auto& instance = Singleton<T>::create_instance(args...);
    EXPECT_TRUE(Singleton<T>::has_instance());

    const T expected{ args... };

    EXPECT_EQ(instance, expected);
}

template <typename T, typename... ARGS>
static void test_double_create_throws(ARGS&&... args) {
    Singleton<T>::create_instance(args...);
    EXPECT_THROW(Singleton<T>::create_instance(args...), std::logic_error);
}

template <typename T, typename... ARGS>
static void test_instance_address(ARGS&&... args) {
    EXPECT_FALSE(Singleton<T>::has_instance());
    auto& instance1 = Singleton<T>::create_instance(args...);
    EXPECT_TRUE(Singleton<T>::has_instance());
    auto& instance2 = Singleton<T>::instance();

    const T expected{ args... };

    EXPECT_EQ(&instance1, &instance2);
    EXPECT_EQ(instance1, expected);
}

template <typename T>
static void test_instance_without_create_throws() {
    EXPECT_FALSE(Singleton<T>::has_instance());
    EXPECT_THROW(Singleton<T>::instance(), std::logic_error);
}

template <typename T, typename... ARGS>
static void test_free_instance(ARGS&&... args) {
    EXPECT_FALSE(Singleton<T>::has_instance());
    Singleton<T>::create_instance(args...);
    EXPECT_TRUE(Singleton<T>::has_instance());
    Singleton<T>::free_instance();
    EXPECT_FALSE(Singleton<T>::has_instance());

    //multiple frees are OK
    EXPECT_NO_THROW(Singleton<T>::free_instance());
}
}

class SingletonTest : public ::testing::Test {
private:
    void TearDown() override {
        free_insts();
    }
};

TEST_F(SingletonTest, create) {
    test_create<int>(42);
    test_create<double>(42.3);
    test_create<char>('a');
    test_create<Object<1>>(1);
    test_create<Object<2>>(1, 2);
    test_create<Object<3>>(1, 2, 3);
    test_create<Object<4>>(1, 2, 3, 4);
    test_create<POD>(POD{ 'a', 1, 42.7 });
}

TEST_F(SingletonTest, double_create_throws) {
    test_double_create_throws<int>(42);
    test_double_create_throws<double>(42.3);
    test_double_create_throws<char>('a');
    test_double_create_throws<Object<1>>(1);
    test_double_create_throws<Object<2>>(1, 2);
    test_double_create_throws<Object<3>>(1, 2, 3);
    test_double_create_throws<Object<4>>(1, 2, 3, 4);
    test_double_create_throws<POD>(POD{ 'a', 1, 42.7 });
}

TEST_F(SingletonTest, instance_address) {
    test_instance_address<int>(42);
    test_instance_address<double>(42.3);
    test_instance_address<char>('a');
    test_instance_address<Object<1>>(1);
    test_instance_address<Object<2>>(1, 2);
    test_instance_address<Object<3>>(1, 2, 3);
    test_instance_address<Object<4>>(1, 2, 3, 4);
    test_instance_address<POD>(POD{ 'a', 1, 42.7 });
}

TEST_F(SingletonTest, instance_without_create_throws) {
    test_instance_without_create_throws<int>();
    test_instance_without_create_throws<double>();
    test_instance_without_create_throws<char>();
    test_instance_without_create_throws<Object<1>>();
    test_instance_without_create_throws<Object<2>>();
    test_instance_without_create_throws<Object<3>>();
    test_instance_without_create_throws<Object<4>>();
    test_instance_without_create_throws<POD>();
}

TEST_F(SingletonTest, free_instance) {
    test_free_instance<int>(42);
    test_free_instance<double>(42.3);
    test_free_instance<char>('a');
    test_free_instance<Object<1>>(1);
    test_free_instance<Object<2>>(1, 2);
    test_free_instance<Object<3>>(1, 2, 3);
    test_free_instance<Object<4>>(1, 2, 3, 4);
    test_free_instance<POD>(POD{ 'a', 1, 42.7 });
}
}
