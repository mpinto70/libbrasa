set(instrument_srcs
    SingletonTest.cpp
    InstrumentedTest.cpp
)

set(instrument_libs
    instrument
)

add_unit_test(
    instrument
    instrument_srcs
    instrument_libs
)
