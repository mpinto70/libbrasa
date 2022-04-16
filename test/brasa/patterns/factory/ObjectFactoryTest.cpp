#include <brasa/patterns/factory/ObjectFactory.h>

#include <gtest/gtest.h>

namespace brasa::pattern::test {
namespace {

struct Base {
    virtual ~Base() noexcept = default;
    virtual int f() const = 0;
};

struct Derived1 final : public Base {
    Derived1(int a, int b) : val_(a * b) {}
    ~Derived1() noexcept override = default;
    int f() const override { return val_; }
    friend bool operator==(const Derived1& x, const Derived1& y) { return x.val_ == y.val_; }

private:
    int val_;
};

struct Derived1Creator final {
    Derived1Creator(int a, int b) : a_(a), b_(b) {}
    std::unique_ptr<Derived1> operator()() { return std::make_unique<Derived1>(a_, b_); }

private:
    int a_;
    int b_;
};

struct Derived2 final : public Base {
    Derived2(int a, int b) : val_(a * b) {}
    ~Derived2() noexcept override = default;
    int f() const override { return val_; }
    friend bool operator==(const Derived2& x, const Derived2& y) { return x.val_ == y.val_; }

private:
    int val_;
};

struct Derived2Creator final {
    Derived2Creator(int a, int b) : a_(a), b_(b) {}
    std::unique_ptr<Derived2> operator()() { return std::make_unique<Derived2>(a_, b_); }

private:
    int a_;
    int b_;
};

enum class Types {
    type1,
    type2,
};

std::string to_string(Types t) {
    return std::to_string(static_cast<int>(t));
}
}

class ObjectFactoryTest : public ::testing::Test {};

TEST_F(ObjectFactoryTest, create) {
    ObjectFactory<Base, Types> factory;
    EXPECT_TRUE(factory.add(Types::type1, Derived1Creator(1, 17)));
    EXPECT_TRUE(factory.add(Types::type2, Derived2Creator(53, 92)));

    auto ptr1 = factory.get(Types::type1);
    auto ptr2 = factory.get(Types::type2);

    EXPECT_NE(dynamic_cast<Derived1*>(ptr1.get()), nullptr);
    EXPECT_NE(dynamic_cast<Derived2*>(ptr2.get()), nullptr);
}

TEST_F(ObjectFactoryTest, remove_has) {
    ObjectFactory<Base, Types> factory;

    EXPECT_FALSE(factory.has(Types::type1));
    EXPECT_FALSE(factory.has(Types::type2));

    EXPECT_TRUE(factory.add(Types::type1, Derived1Creator(1, 17)));

    EXPECT_TRUE(factory.has(Types::type1));
    EXPECT_FALSE(factory.has(Types::type2));

    EXPECT_TRUE(factory.add(Types::type2, Derived2Creator(53, 92)));

    EXPECT_TRUE(factory.has(Types::type1));
    EXPECT_TRUE(factory.has(Types::type2));

    EXPECT_TRUE(factory.remove(Types::type1));

    EXPECT_FALSE(factory.has(Types::type1));
    EXPECT_TRUE(factory.has(Types::type2));

    EXPECT_FALSE(factory.remove(Types::type1)); // repeated attempt

    EXPECT_FALSE(factory.has(Types::type1));
    EXPECT_TRUE(factory.has(Types::type2));

    EXPECT_TRUE(factory.remove(Types::type2));

    EXPECT_FALSE(factory.has(Types::type1));
    EXPECT_FALSE(factory.has(Types::type2));

    EXPECT_FALSE(factory.remove(Types::type2)); // repeated attempt

    EXPECT_FALSE(factory.has(Types::type1));
    EXPECT_FALSE(factory.has(Types::type2));
}

}
