set(singleton_srcs
    SingletonTest.cpp
)

set(singleton_libs
    singleton
)

add_unit_test(
    singleton
    singleton_srcs
    singleton_libs
)
