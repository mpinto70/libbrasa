# target to perform cppcheck

add_custom_target(
    brasa_cppcheck
    COMMAND cppcheck
        --enable=warning
        --enable=style
        --enable=unusedFunction
        --enable=performance
        --enable=portability
        --std=c++14
        --suppress='*:${PROJECT_SOURCE_DIR}/gtest/*'
        --quiet
        --verbose
        -I ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src
)
