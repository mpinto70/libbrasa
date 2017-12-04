# project to allow visualization of struct composition from a binary file

cmake_minimum_required (VERSION 2.8.11)
project (libbrasa)

set(CMAKE_CXX_FLAGS "-Werror -Wall -O3 -finline-functions -std=c++14 -Wno-unused-local-typedefs")

#set (CMAKE_VERBOSE_MAKEFILE on)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src"
                    "${CMAKE_CURRENT_SOURCE_DIR}/gtest/src/gtest/googletest/include"
                    "${CMAKE_CURRENT_SOURCE_DIR}/gtest/src/gtest/googlemock/include"
                    )

# Recurse subdirectories
add_subdirectory (src)
add_subdirectory (test)

################################################################################
# Google Test Setup - BEGIN ####################################################
################################################################################

# We need thread support
find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/master.zip
    PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
set_target_properties(libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

# Create a libgmock target to be used as a dependency by test programs
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)

# Set libgmock properties
set_target_properties(libgmock PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
)

################################################################################
# Google Test Setup - END ######################################################
################################################################################
