cmake_minimum_required(VERSION 3.10)

project(googletest)

# Download and configure GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# Add GoogleTest libraries
add_library(gtest STATIC)
target_link_libraries(gtest PRIVATE gtest_main)

# Set include directories
target_include_directories(gtest SYSTEM PRIVATE ${googletest_SOURCE_DIR}/googletest/include)
target_include_directories(gtest SYSTEM PRIVATE ${googletest_SOURCE_DIR}/googletest)