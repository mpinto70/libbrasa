cmake_minimum_required (VERSION 3.11)
project (libbrasa VERSION 0.9.0 LANGUAGES CXX)

# add support for CTest
include(CTest)
include(FetchContent)

set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS " \
    -Wno-parentheses \
    -Werror \
    -Wall \
    -Wextra \
    -pedantic-errors \
    -O3 \
    -finline-functions \
    -D PROJECT_DIR=\"\\\"${CMAKE_SOURCE_DIR}\\\"\"")

# set (CMAKE_VERBOSE_MAKEFILE on)

set (CMAKE_INSTALL_PREFIX /brasa)

install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/brasa
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h"
)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnitTest.cmake PRIVATE)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CppCheck.cmake)

function(add_lib lib_name sources_var)
    set(lib_full_name brasa_${lib_name})
    add_library(
        ${lib_full_name}
        ${${sources_var}}
    )

    target_include_directories(
        ${lib_full_name}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    )

    install(
        TARGETS ${lib_full_name}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    )
endfunction(add_lib)

# Recurse subdirectories
add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (demos)
