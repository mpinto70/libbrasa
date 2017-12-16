set(buffer_srcs
    CRCTest.cpp
    CircularTest.cpp
)

set(buffer_libs
    buffer
)

add_unit_test(
    buffer
    buffer_srcs
    buffer_libs
)
