
add_executable(
    testchronometer
    ../../gtest_main.cpp
    ChronometerTest.cpp
    NowTest.cpp
    WaiterTest.cpp
    SleepStdTest.cpp
)

target_link_libraries(
    testchronometer
    libgtest
    libgmock
)
