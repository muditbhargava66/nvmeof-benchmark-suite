cmake_minimum_required(VERSION 3.10)

project(spdlog)

# Download and configure spdlog
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.9.2
)
FetchContent_MakeAvailable(spdlog)

# Add spdlog library
add_library(spdlog INTERFACE)
target_include_directories(spdlog SYSTEM INTERFACE ${spdlog_SOURCE_DIR}/include)