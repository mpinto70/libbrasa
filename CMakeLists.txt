cmake_minimum_required (VERSION 2.8.11)
project (libbrasa)

# add support for CTest
enable_testing()
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Werror \
    -Wall \
    -O3 \
    -finline-functions \
    -Wno-unused-local-typedefs \
    -D PROJECT_DIR=\"\\\"${CMAKE_SOURCE_DIR}\\\"\"")

# set (CMAKE_VERBOSE_MAKEFILE on)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set (CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/brasa)

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UnitTest.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CppCheck.cmake)

INSTALL (
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/brasa
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING PATTERN "*.h"
)

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
