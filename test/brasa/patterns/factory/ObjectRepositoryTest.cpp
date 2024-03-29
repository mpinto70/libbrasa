#include <brasa/patterns/factory/ObjectRepository.h>

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

struct Derived2 final : public Base {
    Derived2(int a, int b) : val_(a * b) {}
    ~Derived2() noexcept override = default;
    int f() const override { return val_; }
    friend bool operator==(const Derived2& x, const Derived2& y) { return x.val_ == y.val_; }

private:
    int val_;
};

enum class Types {
    type1,
    type2,
};

std::string to_string(Types t) {
    return std::to_string(static_cast<int>(t));
}
} // namespace

class ObjectRepositoryTest : public ::testing::Test {};

TEST_F(ObjectRepositoryTest, create_with_unique_ptr) {
    ObjectRepository<Base, Types> repo;
    auto ptr1 = new Derived1(1, 17);
    auto ptr2 = new Derived2(53, 92);
    EXPECT_TRUE(repo.add(Types::type1, std::unique_ptr<Derived1>(ptr1)));
    EXPECT_TRUE(repo.add(Types::type2, std::unique_ptr<Derived2>(ptr2)));
    EXPECT_FALSE(repo.add(Types::type1, std::make_unique<Derived1>(2, 3)));
    EXPECT_FALSE(repo.add(Types::type2, std::make_unique<Derived2>(12, 42)));

    EXPECT_EQ(ptr1, &repo.get(Types::type1));
    EXPECT_EQ(ptr2, &repo.get(Types::type2));
}

TEST_F(ObjectRepositoryTest, create_with_params) {
    ObjectRepository<Base, Types> repo;
    EXPECT_TRUE(repo.template add<Derived1>(Types::type1, 28, 90));
    EXPECT_TRUE(repo.template add<Derived2>(Types::type2, 13, 32));
    EXPECT_FALSE(repo.add(Types::type1, std::make_unique<Derived1>(2, 3)));
    EXPECT_FALSE(repo.add(Types::type2, std::make_unique<Derived2>(12, 42)));

    auto ptr1 = dynamic_cast<Derived1*>(&repo.get(Types::type1));
    auto ptr2 = dynamic_cast<Derived2*>(&repo.get(Types::type2));

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_EQ(*ptr1, Derived1(28, 90));
    EXPECT_EQ(*ptr2, Derived2(13, 32));
}

TEST_F(ObjectRepositoryTest, remove_has) {
    ObjectRepository<Base, Types> repo;

    EXPECT_FALSE(repo.has(Types::type1));
    EXPECT_FALSE(repo.has(Types::type2));

    EXPECT_TRUE(repo.add(Types::type1, std::make_unique<Derived1>(2, 3)));

    EXPECT_TRUE(repo.has(Types::type1));
    EXPECT_FALSE(repo.has(Types::type2));

    EXPECT_TRUE(repo.add(Types::type2, std::make_unique<Derived2>(12, 42)));

    EXPECT_TRUE(repo.has(Types::type1));
    EXPECT_TRUE(repo.has(Types::type2));

    EXPECT_TRUE(repo.remove(Types::type1));

    EXPECT_FALSE(repo.has(Types::type1));
    EXPECT_TRUE(repo.has(Types::type2));

    EXPECT_TRUE(repo.remove(Types::type2));

    EXPECT_FALSE(repo.has(Types::type1));
    EXPECT_FALSE(repo.has(Types::type2));
}

} // namespace brasa::pattern::test
