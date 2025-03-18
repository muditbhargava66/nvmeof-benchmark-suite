# CMake Configuration Files

This directory contains CMake modules and configuration files used by the NVMe-oF Benchmarking Suite.

## Files

- `nvmeof_benchmarking_suite-config.cmake.in`: Template for generating package configuration files
- `FindSPDK.cmake`: Module to find and configure SPDK libraries
- `CompilationSettings.cmake`: Common compilation settings and flags

## Usage

### In the Main CMakeLists.txt

Include these modules in the main CMakeLists.txt file:

```cmake
# Include common compilation settings
include(cmake/CompilationSettings.cmake)

# Find SPDK
include(cmake/FindSPDK.cmake)
```

### Package Configuration

The `nvmeof_benchmarking_suite-config.cmake.in` file is used to generate a package configuration file when installing the project. This allows other projects to find and use the NVMe-oF Benchmarking Suite with CMake's `find_package` command.

To generate the package configuration:

```cmake
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
```

## Adding New Modules

When adding new CMake modules:

1. Create the module file with a `.cmake` extension
2. Document the module with comments
3. Update this README.md file
4. Include the module in the main CMakeLists.txt file

## Customizing Compilation Settings

You can customize the compilation settings by modifying `CompilationSettings.cmake` or by setting CMake variables before including it:

```cmake
set(ENABLE_WERROR ON)
include(cmake/CompilationSettings.cmake)
```

## Finding SPDK

The `FindSPDK.cmake` module allows for flexible configuration of the SPDK library. You can specify the SPDK location using:

```cmake
# Option 1: Set an environment variable
export SPDK_DIR=/path/to/spdk

# Option 2: Pass it as a CMake variable
cmake -DSPDK_DIR=/path/to/spdk ..

# Option 3: Use the default search paths
# (The module will search in common locations)
```

On macOS, if `USE_MOCK_SPDK` is set to ON, the module will look for the mock implementation in `third_party/spdk_mock`.