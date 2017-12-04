
add_library(
    instrument
    Instrumented.cpp
)

target_include_directories(
    instrument
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
