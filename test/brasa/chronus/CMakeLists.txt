
add_executable(
    testchronus
    ../../gtest_main.cpp
    ChronometerTest.cpp
    NowTest.cpp
    WaiterTest.cpp
    SleepStdTest.cpp
)

target_link_libraries(
    testchronus
    libgtest
    libgmock
)
