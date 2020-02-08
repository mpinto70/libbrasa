
add_executable(
    argparse_demo
    argparse.cpp
)

target_link_libraries(
    argparse_demo
    PRIVATE brasa_argparse
)
