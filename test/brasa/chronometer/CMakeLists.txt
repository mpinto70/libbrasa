
add_executable(
    testchronometer
    ../../gtest_main.cpp
    ChronometerTest.cpp
)

target_link_libraries(
    testchronometer
    libgtest
    libgmock
)
