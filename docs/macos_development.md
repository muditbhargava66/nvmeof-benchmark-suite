# macOS Development Guide for NVMe-oF Benchmarking Suite

This guide explains how to set up and develop the NVMe-oF Benchmarking Suite on macOS. Since SPDK (Storage Performance Development Kit) does not natively support macOS, we provide a mock implementation for development and testing on macOS, while using Docker for actual benchmarking.

## Development Options on macOS

You have two main options for development on macOS:

1. **Mock Implementation**: Use our SPDK mock implementation for compiling and testing on macOS directly.
2. **Docker Development Environment**: Use a Docker container with the full Linux environment and real SPDK.

## Option 1: Using the SPDK Mock Implementation

### Prerequisites

- macOS 10.15 (Catalina) or newer
- Xcode Command Line Tools
- CMake 3.14 or newer
- Git

### Setup Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

2. Run the SPDK installation script:
   ```bash
   chmod +x scripts/install_spdk.sh
   ./scripts/install_spdk.sh
   ```
   This will set up the SPDK mock implementation in the `third_party/spdk_mock` directory.

3. Create a build directory and configure the project:
   ```bash
   mkdir -p build && cd build
   cmake ..
   ```

4. Build the project:
   ```bash
   make
   ```

5. Run the tests:
   ```bash
   make test
   ```

### Limitations of the Mock Implementation

The SPDK mock implementation provides the minimal API required for the NVMe-oF Benchmarking Suite to compile and run basic tests on macOS. It does not:

- Perform actual NVMe operations
- Connect to real NVMe devices
- Provide accurate performance measurements

It's intended for development of the benchmarking suite's structure, UI, and non-NVMe components only.

## Option 2: Using Docker Development Environment

For a more complete development environment with real SPDK functionality:

### Prerequisites

- Docker Desktop for Mac
- Git

### Setup Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/muditbhargava66/nvmeof-benchmark-suite.git
   cd nvmeof-benchmark-suite
   ```

2. Build and start the development container:
   ```bash
   docker-compose build nvmeof-development
   docker-compose up -d nvmeof-development
   ```

3. Connect to the container:
   ```bash
   docker-compose exec nvmeof-development bash
   ```

4. Inside the container, build the project:
   ```bash
   cd /app/build
   cmake ..
   make
   ```

5. Run tests inside the container:
   ```bash
   make test
   ```

### Running Benchmarks

To run actual benchmarks, you'll need to use the benchmarking container on a Linux host with NVMe devices:

```bash
docker-compose build nvmeof-benchmarking
docker-compose up nvmeof-benchmarking
```

Note that NVMe device access requires privileged mode and will not work on macOS hosts due to virtualization limitations in Docker Desktop for Mac.

## Switching Between Development Environments

You can switch between local macOS development (with the mock SPDK) and Docker-based development as needed:

- Use macOS with mock SPDK for UI development, general code structure, and non-NVMe components
- Use Docker for testing with real SPDK functionality and performance measurement logic

## Submitting Changes

When submitting changes, please ensure:

1. Your code compiles with both the mock SPDK on macOS and real SPDK in the Docker environment
2. Tests pass in both environments
3. You've tested any SPDK-dependent functionality in the Docker environment

## Troubleshooting

### CMake Cannot Find SPDK

If CMake cannot find SPDK on macOS, ensure the mock implementation is correctly set up:

```bash
ls -la third_party/spdk_mock/include/spdk/nvme.h
```

If the file doesn't exist, re-run the installation script:

```bash
./scripts/install_spdk.sh
```

### Build Errors with Mock SPDK

If you encounter build errors related to the mock SPDK implementation:

1. Check that the mock implementation includes all required functions
2. Compare error messages with the actual SPDK API documentation
3. Add any missing functions to the mock implementation

### Docker Issues

If you have issues with the Docker development environment:

1. Ensure Docker Desktop is running
2. Check Docker Desktop has sufficient resources allocated
3. Try rebuilding the container:
   ```bash
   docker-compose build --no-cache nvmeof-development
   ```

## Additional Resources

- [SPDK Documentation](https://spdk.io/doc/)
- [Docker Desktop for Mac Documentation](https://docs.docker.com/desktop/mac/)