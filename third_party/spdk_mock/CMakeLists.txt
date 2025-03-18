cmake_minimum_required(VERSION 3.14)

project(spdk_mock
        VERSION 1.0.0
        DESCRIPTION "Mock SPDK implementation for macOS development"
        LANGUAGES C)

# Create the mock SPDK library
add_library(spdk_mock STATIC
    src/nvme.c
)

# Set include directories
target_include_directories(spdk_mock
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set properties
set_target_properties(spdk_mock PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Install targets
install(TARGETS spdk_mock
    EXPORT spdk_mock-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

# Install header files
install(DIRECTORY include/
    DESTINATION include
)