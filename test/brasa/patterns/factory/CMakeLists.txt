set(factory_srcs
    ObjectRepositoryTest.cpp
)

set(factory_libs
    factory
)

add_unit_test(
    factory
    factory_srcs
    factory_libs
)
