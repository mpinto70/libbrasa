set(thread_srcs
    RcuTest.cpp
    RcuReaderTest.cpp
    RcuWriterTest.cpp
)

set(thread_libs
    thread
)

add_unit_test(
    thread
    thread_srcs
    thread_libs
)
