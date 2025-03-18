# Changelog

All notable changes to the NVMe-oF Benchmarking Suite will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2025-03-18

### Added
- Comprehensive unit testing framework using Google Test
- Integration tests for end-to-end validation
- Code coverage reporting capabilities
- Enhanced CMake configuration with better organization
- Detailed code style guide and contribution guidelines
- Thread safety improvements with proper mutex usage
- Robust error handling throughout the codebase
- New utility functions for file and string operations
- System information utilities for hardware detection
- Command-line argument parsing with support for various options
- Signal handling for graceful shutdown
- Resource usage monitoring with callback support
- Performance optimization recommendations based on bottleneck detection
- Support for multiple output formats including CSV, JSON, and plaintext
- Enhanced documentation with Doxygen-style comments
- macOS development support with platform-specific implementations
- SPDK mock implementation for development without NVMe hardware
- Cross-platform compatibility between Linux and macOS
- Docker support for development and testing environments
- Dynamic workload generation based on JSON profiles
- Real-time resource monitoring and bottleneck detection
- Visualization tools for benchmark results
- Improved build scripts with better organization and functionality
  - Enhanced setup.sh for one-step environment configuration
  - Improved build.sh with better platform detection
  - New run_tests.sh script for streamlined testing
  - New visualize_results.sh script for easier result visualization
  - Consistent error handling and command-line options across all scripts
- Enhanced documentation structure
  - Added FAQ.md for frequently asked questions
  - Added TROUBLESHOOTING.md for common issues
  - Added INSTALL.md with detailed installation instructions
  - Added CODE_OF_CONDUCT.md for community guidelines
  - Improved README.md in data and docs directories

### Changed
- Migrated to C++17 standard for improved language features
- Reorganized project structure for better maintainability
- Enhanced build system with more flexibility and options
- Improved memory management with RAII principles
- Updated file operations to use std::filesystem
- Modernized code with smart pointers and modern C++ practices
- Enhanced WorkloadProfile validation with more comprehensive checks
- Upgraded resource monitoring with better metrics collection
- Improved bottleneck detection with more accurate thresholds
- Enhanced data collection with structured data points
- Better result visualization with more detailed output
- Platform-specific conditional compilation for cross-platform support
- More modular architecture with clear separation of concerns
- Restructured data directory with better organization
  - Standardized naming conventions for data files
  - Added README.md with usage instructions
  - Improved file formats for better interoperability

### Fixed
- Thread safety issues in file operations
- Resource leaks in NVMe operations
- Memory leaks in workload generation
- Race conditions in resource monitoring
- Improper error handling in configuration loading
- Lack of input validation for user-provided parameters
- Compiler warnings related to sign comparisons and unused parameters
- Type qualifier inconsistencies in SPDK mock interface
- Namespace issues with forward declarations
- Platform-specific compatibility issues for macOS development
- Memory errors (double free) in DataCollector tests by proper object lifecycle management
- Improved ResourceMonitor test to correctly handle non-copyable std::atomic members
- Fixed unused variable warnings in multiple test files
- Corrected implementation of ConfigApplicator with proper virtual methods
- Fixed sign comparison warnings in ResultVisualizer
- Enhanced mock object design for better testing of optimization components
- Improved CSV parsing in ResultVisualizer for more robust data handling
- Better error message handling in file operations
- Fixed deprecated method usage with proper compiler directives
- SPDK integration issues with latest versions
- Integration test failures on macOS
  - Fixed directory creation in tests
  - Added macOS-specific error handling
  - Improved SetSysctlValue method for cross-platform support
  - Fixed bottleneck detection tests for macOS
- Fixed override keyword usage in mock objects
- Added virtual keyword to overridden methods

### Security
- Improved input validation for all user-provided data
- Enhanced file access controls
- Better protection against resource exhaustion
- Proper signal handling for controlled shutdown

## [1.0.0] - 2024-05-30

### Added
- Initial release of NVMe-oF Benchmarking Suite
- Basic workload generation capabilities
- Performance metric collection
- Simple bottleneck analysis
- Basic optimization recommendations
- Result visualization and reporting
- Command-line interface
- Basic documentation

[2.0.0]: https://github.com/muditbhargava66/nvmeof-benchmark-suite/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/muditbhargava66/nvmeof-benchmark-suite/releases/tag/v1.0.0