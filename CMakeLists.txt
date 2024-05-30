cmake_minimum_required(VERSION 3.10)

project(nvmeof_benchmarking_suite)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add third-party libraries
add_subdirectory(third_party)

# Add source directories
add_subdirectory(src)

# Add test directories
enable_testing()
add_subdirectory(tests)

# Add install target
install(TARGETS nvmeof_benchmarking nvmeof_analysis
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Install header files
install(DIRECTORY include/ DESTINATION include)

# Install data files
install(DIRECTORY data/ DESTINATION data)

# Install scripts
install(DIRECTORY scripts/ DESTINATION scripts
        FILES_MATCHING PATTERN "*.sh")

# Generate package configuration files
include(CMakePackageConfigHelpers)
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
install(FILES
    "${PROJECT_BINARY_DIR}/nvmeof_benchmarking_suite-config.cmake"
    "${PROJECT_BINARY_DIR}/nvmeof_benchmarking_suite-config-version.cmake"
    DESTINATION lib/cmake/nvmeof_benchmarking_suite
)

# Export targets
install(EXPORT nvmeof_benchmarking_suite-targets
        FILE nvmeof_benchmarking_suite-targets.cmake
        NAMESPACE nvmeof_benchmarking_suite::
        DESTINATION lib/cmake/nvmeof_benchmarking_suite
)