
add_executable(
    testinstrument
    ../../gtest_main.cpp
    SingletonTest.cpp
)

target_link_libraries(
    testinstrument
    libgtest
    libgmock
)
