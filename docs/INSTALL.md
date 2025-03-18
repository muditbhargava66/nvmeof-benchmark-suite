# Installation Guide

This document provides detailed instructions for installing the NVMe-oF Benchmarking Suite on various platforms.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Linux Installation](#linux-installation)
   - [Ubuntu/Debian](#ubuntudebian)
   - [CentOS/RHEL](#centosrhel)
   - [Fedora](#fedora)
3. [macOS Development Setup](#macos-development-setup)
4. [Docker Installation](#docker-installation)
5. [Building from Source](#building-from-source)
6. [Verifying Installation](#verifying-installation)
7. [Troubleshooting](#troubleshooting)

## Prerequisites

### Common Requirements

- C++17 compatible compiler
- CMake 3.14 or newer
- Git
- SPDK libraries (will be installed by our script)

### Linux-Specific Requirements

- Linux kernel 4.15 or newer (recommended 5.4+)
- Development tools (build-essential or equivalent)
- libnuma development libraries
- libssl development libraries
- libaio development libraries

### macOS-Specific Requirements

- macOS 10.15 (Catalina) or newer
- Xcode Command Line Tools
- Homebrew (recommended)

## Linux Installation

### Ubuntu/Debian

1. Update your system and install dependencies:

   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake git libnuma-dev libssl-dev libaio-dev
   ```

2. Clone the repository:

   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

3. Run the setup script:

   ```bash
   chmod +x scripts/setup.sh
   ./scripts/setup.sh
   ```

   This script will:
   - Install SPDK if not already installed
   - Create necessary directories
   - Generate sample configuration files
   - Build the project

4. Alternatively, you can perform the installation steps manually:

   ```bash
   # Install SPDK
   ./scripts/install_spdk.sh
   
   # Build the project
   mkdir -p build && cd build
   cmake ..
   make -j$(nproc)
   
   # Run tests to verify installation
   make test
   ```

### CentOS/RHEL

1. Install dependencies:

   ```bash
   sudo yum groupinstall -y "Development Tools"
   sudo yum install -y cmake3 numactl-devel openssl-devel libaio-devel
   
   # Create cmake symlink if using cmake3
   sudo ln -sf /usr/bin/cmake3 /usr/bin/cmake
   ```

2. Clone the repository:

   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

3. Run the setup script:

   ```bash
   chmod +x scripts/setup.sh
   ./scripts/setup.sh
   ```

### Fedora

1. Install dependencies:

   ```bash
   sudo dnf install -y gcc-c++ cmake git make numactl-devel openssl-devel libaio-devel
   ```

2. Clone the repository:

   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

3. Run the setup script:

   ```bash
   chmod +x scripts/setup.sh
   ./scripts/setup.sh
   ```

## macOS Development Setup

Since SPDK is not natively supported on macOS, we provide a mock implementation for development and testing.

1. Install dependencies using Homebrew:

   ```bash
   brew install cmake gcc make
   ```

2. Clone the repository:

   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

3. Run the setup script:

   ```bash
   chmod +x scripts/setup.sh
   ./scripts/setup.sh
   ```

   This will set up the mock SPDK implementation automatically.

4. Alternatively, you can perform the setup manually:

   ```bash
   # Set up mock SPDK
   ./scripts/install_spdk.sh
   
   # Build the project
   mkdir -p build && cd build
   cmake ..
   make -j$(sysctl -n hw.ncpu)
   
   # Run tests to verify installation
   make test
   ```

For more details on macOS development, see [macOS Development Guide](macos_development.md).

## Docker Installation

Docker provides a consistent environment for development and testing.

1. Install Docker and Docker Compose:
   - [Docker Desktop for Mac](https://docs.docker.com/desktop/mac/install/)
   - [Docker Desktop for Windows](https://docs.docker.com/desktop/windows/install/)
   - [Docker Engine for Linux](https://docs.docker.com/engine/install/)

2. Clone the repository:

   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

3. Build and run the development container:

   ```bash
   docker-compose build nvmeof-development
   docker-compose up -d nvmeof-development
   ```

4. Enter the container:

   ```bash
   docker-compose exec nvmeof-development bash
   ```

5. Inside the container, build the project:

   ```bash
   cd /app/build
   cmake ..
   make -j$(nproc)
   ```

## Building from Source

If you prefer not to use the setup script, you can build the project manually:

1. Clone the repository:

   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

2. Install SPDK:

   ```bash
   ./scripts/install_spdk.sh
   ```

3. Create a build directory and configure:

   ```bash
   mkdir -p build && cd build
   ```

4. Run CMake with your preferred options:

   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
   ```

   Available CMake options:
   - `-DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo|MinSizeRel`
   - `-DBUILD_TESTS=ON|OFF`
   - `-DBUILD_DOCS=ON|OFF`
   - `-DENABLE_SANITIZERS=ON|OFF`
   - `-DUSE_STATIC_ANALYSIS=ON|OFF`
   - `-DUSE_MOCK_SPDK=ON|OFF` (automatically set to ON for macOS)

5. Build the project:

   ```bash
   make -j$(nproc)  # Linux
   make -j$(sysctl -n hw.ncpu)  # macOS
   ```

6. Optionally install:

   ```bash
   sudo make install
   ```

## Verifying Installation

To verify that the installation was successful:

1. Run the tests:

   ```bash
   cd build
   make test
   ```

2. Run a simple benchmark:

   ```bash
   ./bin/nvmeof_benchmarking --workload-profile ../data/workload_profiles/workload_profile_1.json --output-dir ../data/benchmark_results
   ```

3. Check that the results were generated:

   ```bash
   ls -la ../data/benchmark_results/
   ```

## Troubleshooting

If you encounter issues during installation, please refer to the [Troubleshooting Guide](TROUBLESHOOTING.md) for common issues and solutions.

Common issues include:

- Missing dependencies
- Insufficient permissions
- Compiler version compatibility
- SPDK installation problems
- CMake configuration errors

If you still have issues, please [open an issue](https://github.com/muditbhargava66/nvmeof-benchmark-suite/issues) on our GitHub repository.