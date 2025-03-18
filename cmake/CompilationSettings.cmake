# CompilationSettings.cmake - Common compilation settings for the project
#
# This file contains compiler flags, warning settings, and other build options
# that are applied to all targets in the project.

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Apply compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    # Base flags for better optimization and debugging
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    
    # Add additional flags based on build type
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
    endif()
    
    # Additional warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wcast-align -Wunused -Wconversion -Wsign-conversion")
    
    # Enable more warnings for GCC
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wduplicated-branches -Wlogical-op")
        # Add -Wuseless-cast for GCC >= 4.8
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.8)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wuseless-cast")
        endif()
    endif()
    
    # Enable more warnings for Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdocumentation -Wno-documentation-unknown-command")
    endif()
    
    # Conditional flags
    option(ENABLE_WERROR "Treat warnings as errors" OFF)
    if(ENABLE_WERROR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif()
endif()

# MSVC-specific flags (if ever needed)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /utf-8")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    
    if(ENABLE_WERROR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    endif()
endif()

# Configure sanitizers for Debug builds
option(ENABLE_SANITIZERS "Enable sanitizers in debug builds" OFF)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        message(STATUS "Enabling sanitizers for debug build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
    else()
        message(WARNING "Sanitizers requested but not supported with this compiler")
    endif()
endif()

# Configure code coverage for Debug builds
option(ENABLE_COVERAGE "Enable code coverage in debug builds" OFF)
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling code coverage for debug build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
    else()
        message(WARNING "Code coverage requested but not supported with this compiler")
    endif()
endif()

# Configure static analysis
option(USE_STATIC_ANALYSIS "Enable static analysis during compilation" OFF)
if(USE_STATIC_ANALYSIS)
    # Configure clang-tidy if available
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Static analysis enabled with clang-tidy")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=*,-llvm-header-guard,-fuchsia-*,-google-*,-android-*,-abseil-*,-objc-*,-readability-magic-numbers,-cppcoreguidelines-avoid-magic-numbers")
    else()
        message(WARNING "Static analysis requested but clang-tidy not found")
    endif()
endif()

# Use Link Time Optimization for Release builds
option(ENABLE_LTO "Enable Link Time Optimization in release builds" OFF)
if(ENABLE_LTO AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
    if(IPO_SUPPORTED)
        message(STATUS "Link Time Optimization enabled for release build")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "Link Time Optimization requested but not supported: ${IPO_ERROR}")
    endif()
endif()

# Output compilation settings
message(STATUS "Compilation settings:")
message(STATUS "  C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "  Sanitizers: Enabled")
else()
    message(STATUS "  Sanitizers: Disabled")
endif()
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "  Code coverage: Enabled")
else()
    message(STATUS "  Code coverage: Disabled")
endif()
if(USE_STATIC_ANALYSIS AND CLANG_TIDY_EXE)
    message(STATUS "  Static analysis: Enabled (clang-tidy)")
else()
    message(STATUS "  Static analysis: Disabled")
endif()
if(ENABLE_LTO AND IPO_SUPPORTED AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "  Link Time Optimization: Enabled")
else()
    message(STATUS "  Link Time Optimization: Disabled")
endif()
