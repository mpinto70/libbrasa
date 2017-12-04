
add_executable(
    testinstrument
    ../../gtest_main.cpp
    SingletonTest.cpp
    InstrumentedTest.cpp
)

target_link_libraries(
    testinstrument
    instrument
    libgtest
    libgmock
)
