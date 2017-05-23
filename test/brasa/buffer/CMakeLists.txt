
add_executable(
    testbuffer
    ../../gtest_main.cpp
    CRCTest.cpp
    CircularTest.cpp
    CircularReaderTest.cpp
)

target_link_libraries(
    testbuffer
    buffer
    libgtest
    libgmock
)
