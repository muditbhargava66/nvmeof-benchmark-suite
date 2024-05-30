cmake_minimum_required(VERSION 3.10)

project(third_party)

# GoogleTest
add_subdirectory(googletest)

# spdlog
add_subdirectory(spdlog)

# nlohmann/json
add_subdirectory(nlohmann_json)