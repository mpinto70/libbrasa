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

set(copy_benchmark_srcs
    CopyBenchmark.cpp
)

set (copy_benchmark_libs
    buffer
)

add_benchmark_test(
    copy
    copy_benchmark_srcs
    copy_benchmark_libs
)

