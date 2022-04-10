set(factory_srcs
    ObjectRepositoryTest.cpp
    ObjectFactoryTest.cpp
)

set(factory_libs
    factory
)

add_unit_test(
    factory
    factory_srcs
    factory_libs
)
