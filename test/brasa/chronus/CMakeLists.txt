set(chronus_srcs
    ChronometerTest.cpp
    NowTest.cpp
    WaiterTest.cpp
    SleepStdTest.cpp
)

set(chronus_libs
    chronus
)

add_unit_test(
    chronus
    chronus_srcs
    chronus_libs
)
