cmake_minimum_required(VERSION 3.14)

project(nvmeof_benchmarking_suite
        VERSION 2.0.0
        DESCRIPTION "NVMe-oF Benchmarking Suite"
        HOMEPAGE_URL "https://github.com/muditbhargava66/nvmeof-benchmark-suite"
        LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compile options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define options
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_DOCS "Build the documentation" OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers in debug builds" OFF)
option(USE_STATIC_ANALYSIS "Enable static analysis during compilation" OFF)
option(USE_MOCK_SPDK "Use mock SPDK implementation" ${APPLE})

# Configure sanitizers
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE MATCHES "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling sanitizers for debug build")
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    else()
        message(WARNING "Sanitizers requested but not supported with this compiler")
    endif()
endif()

# Find required packages
find_package(Threads REQUIRED)

# Add third-party libraries
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/CMakeLists.txt")
    add_subdirectory(third_party)
endif()

# Check if SPDK was found by the third_party CMakeLists.txt
if(NOT SPDK_FOUND)
    if(APPLE)
        message(STATUS "On macOS: Using SPDK mock implementation")
        set(USE_MOCK_SPDK ON)
        
        # Set up the SPDK mock
        add_subdirectory(third_party/spdk_mock)
        set(SPDK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdk_mock/include)
        set(SPDK_LIBRARIES spdk_mock)
    else()
        message(FATAL_ERROR "SPDK not found. Please install SPDK or run the install_spdk.sh script.")
    endif()
else()
    message(STATUS "Using SPDK from: ${SPDK_INCLUDE_DIRS}")
endif()

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SPDK_INCLUDE_DIRS}
)

# Add source directories
add_subdirectory(src)

# Configure testing
if(BUILD_TESTS)
    enable_testing()
    include(GoogleTest)
    
    # Add GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Add test directories
    add_subdirectory(tests)
endif()

# Configure documentation
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()
        message(STATUS "Doxygen not found, documentation will not be built")
    endif()
endif()

# Generate package configuration files
option(GENERATE_PACKAGE_CONFIG "Generate package configuration files" OFF)

if(GENERATE_PACKAGE_CONFIG)
  include(CMakePackageConfigHelpers)
  
  if(EXISTS "${PROJECT_SOURCE_DIR}/cmake/nvmeof_benchmarking_suite-config.cmake.in")
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/nvmeof_benchmarking_suite-config.cmake.in"
        "${PROJECT_BINARY_DIR}/nvmeof_benchmarking_suite-config.cmake"
        INSTALL_DESTINATION lib/cmake/nvmeof_benchmarking_suite
    )

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/nvmeof_benchmarking_suite-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    # Install targets
    install(EXPORT nvmeof_benchmarking_suite-targets
            FILE nvmeof_benchmarking_suite-targets.cmake
            NAMESPACE nvmeof_benchmarking_suite::
            DESTINATION lib/cmake/nvmeof_benchmarking_suite
    )

    install(FILES
        "${PROJECT_BINARY_DIR}/nvmeof_benchmarking_suite-config.cmake"
        "${PROJECT_BINARY_DIR}/nvmeof_benchmarking_suite-config-version.cmake"
        DESTINATION lib/cmake/nvmeof_benchmarking_suite
    )
  else()
    message(STATUS "Package configuration template not found - skipping package config generation")
  endif()
endif()

# Print build configuration summary
message(STATUS "")
message(STATUS "NVMe-oF Benchmarking Suite Configuration:")
message(STATUS "  Version:              ${PROJECT_VERSION}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build tests:          ${BUILD_TESTS}")
message(STATUS "  Build documentation:  ${BUILD_DOCS}")
message(STATUS "  Sanitizers:           ${ENABLE_SANITIZERS}")
message(STATUS "  Static analysis:      ${USE_STATIC_ANALYSIS}")
message(STATUS "  Using mock SPDK:      ${USE_MOCK_SPDK}")
message(STATUS "")