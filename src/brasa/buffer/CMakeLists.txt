
add_library(
    buffer
    CRC.cpp
)

target_include_directories(
    buffer
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
