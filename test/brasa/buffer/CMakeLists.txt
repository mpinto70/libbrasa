set(buffer_srcs
    CircularTest.cpp
    CopyTest.cpp
    CRCTest.cpp
)

set(buffer_libs
    buffer
)

add_unit_test(
    buffer
    buffer_srcs
    buffer_libs
)
