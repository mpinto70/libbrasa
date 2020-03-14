
add_executable(
    buffer_demo
    buffer.cpp
)

target_link_libraries(
    buffer_demo
    PRIVATE brasa_buffer
    PRIVATE pthread
    PRIVATE rt
)
