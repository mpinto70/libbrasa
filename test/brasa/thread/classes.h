#pragma once

#include <type_traits>

namespace brasa::thread::test {

class NonMoveable {
public:
    NonMoveable() = default;
    NonMoveable(const NonMoveable&) = default;
    NonMoveable(NonMoveable&&) = delete;
    NonMoveable& operator=(const NonMoveable&) = default;
    NonMoveable& operator=(NonMoveable&&) = delete;
};

static_assert(std::is_move_constructible_v<NonMoveable> == false);
static_assert(std::is_move_assignable_v<NonMoveable> == false);

} // namespace brasa::thread::test
