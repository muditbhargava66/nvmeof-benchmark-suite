# Troubleshooting Guide

This document provides solutions for common issues that users may encounter when using the NVMe-oF Benchmarking Suite.

## Table of Contents

1. [Installation Issues](#installation-issues)
2. [Build Issues](#build-issues)
3. [Runtime Issues](#runtime-issues)
4. [Performance Issues](#performance-issues)
5. [macOS-Specific Issues](#macos-specific-issues)
6. [Docker-Specific Issues](#docker-specific-issues)

## Installation Issues

### SPDK Installation Fails

**Issue**: The SPDK installation script fails with errors.

**Solutions**:
- Ensure you have the necessary dependencies installed:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install -y build-essential libnuma-dev libssl-dev libaio-dev
  
  # CentOS/RHEL
  sudo yum install -y gcc gcc-c++ make numactl-devel openssl-devel libaio-devel
  ```
- Verify that you have sufficient permissions:
  ```bash
  chmod +x scripts/install_spdk.sh
  sudo ./scripts/install_spdk.sh
  ```
- Check network connectivity if the script fails to download SPDK.

### CMake Not Found

**Issue**: The `cmake` command is not found.

**Solution**:
- Install CMake:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install -y cmake
  
  # CentOS/RHEL
  sudo yum install -y cmake3
  
  # macOS
  brew install cmake
  ```

## Build Issues

### Compiler Does Not Support C++17

**Issue**: The compiler reports that it does not support C++17 features.

**Solutions**:
- Install a newer compiler:
  ```bash
  # Ubuntu/Debian
  sudo apt-get install -y g++-9
  export CXX=g++-9
  
  # macOS
  brew install gcc@9
  export CXX=g++-9
  ```
- Update your existing compiler.

### Missing SPDK Headers

**Issue**: The build fails with missing SPDK header errors.

**Solutions**:
- Ensure SPDK is properly installed:
  ```bash
  ./scripts/install_spdk.sh
  ```
- Verify that the SPDK installation path is correctly set in CMake:
  ```bash
  cd build
  cmake .. -DSPDK_DIR=/path/to/spdk
  ```
- On macOS, ensure the mock SPDK implementation is in place:
  ```bash
  ls -la third_party/spdk_mock/include/spdk/
  ```

### Link Errors

**Issue**: The build fails with undefined reference errors.

**Solutions**:
- Check that all dependencies are installed.
- Ensure that the SPDK libraries are properly linked in CMake.
- Rebuild the project from scratch:
  ```bash
  rm -rf build
  mkdir build && cd build
  cmake ..
  make
  ```

## Runtime Issues

### Cannot Find NVMe Devices

**Issue**: The application cannot find or access NVMe devices.

**Solutions**:
- Ensure you have NVMe devices installed on your system:
  ```bash
  lspci | grep -i nvme
  ```
- Verify that the user has sufficient permissions to access the devices:
  ```bash
  sudo usermod -a -G disk $USER
  # Log out and log back in
  ```
- Check that the NVMe devices are not already in use by other applications.

### Connection Failures

**Issue**: The application fails to connect to NVMe-oF targets.

**Solutions**:
- Verify that the NVMe-oF target is running and accessible:
  ```bash
  ping <target-ip>
  ```
- Check the target configuration in `data/target_configs/`.
- Ensure that the required ports are open in the firewall:
  ```bash
  # For RDMA
  sudo ufw allow 4420/tcp
  
  # For TCP
  sudo ufw allow 4420/tcp
  ```

### Data Collection Issues

**Issue**: The application fails to collect performance data or writes corrupted data.

**Solutions**:
- Check that the output directory exists and is writable:
  ```bash
  mkdir -p data/benchmark_results
  chmod 755 data/benchmark_results
  ```
- Verify the workload profile is correctly formatted JSON.
- Run with verbose output to see detailed logging:
  ```bash
  ./build/bin/nvmeof_benchmarking --workload-profile ... --verbose
  ```

## Performance Issues

### Poor Benchmark Performance

**Issue**: The benchmark performance is significantly lower than expected.

**Solutions**:
- Check system resource utilization during benchmarking:
  ```bash
  top
  ```
- Verify that the system is not running other resource-intensive applications.
- Check for CPU throttling:
  ```bash
  sudo cpupower frequency-info
  ```
- Ensure that the NVMe-oF transport is properly configured:
  ```bash
  modprobe nvme_rdma  # For RDMA transport
  modprobe nvme_tcp   # For TCP transport
  ```

### High CPU Usage

**Issue**: The application uses excessive CPU resources.

**Solutions**:
- Adjust the workload profile for lower intensity:
  ```json
  {
    "interval_us": 1000,  // Increase interval between operations
    "num_threads": 4      // Reduce number of threads
  }
  ```
- Use CPU affinity to bind the application to specific cores:
  ```bash
  taskset -c 0-3 ./build/bin/nvmeof_benchmarking ...
  ```

### High Memory Usage

**Issue**: The application uses excessive memory.

**Solutions**:
- Reduce the workload buffer size in the profile:
  ```json
  {
    "buffer_size": 65536  // Reduce buffer size
  }
  ```
- Limit the number of concurrent operations.

## macOS-Specific Issues

### Mock SPDK Implementation Not Found

**Issue**: The mock SPDK implementation is not found on macOS.

**Solutions**:
- Run the installation script specifically for macOS:
  ```bash
  ./scripts/install_spdk.sh
  ```
- Manually set up the mock implementation directory:
  ```bash
  mkdir -p third_party/spdk_mock/include/spdk
  mkdir -p third_party/spdk_mock/src
  ```

### Integration Tests Fail on macOS

**Issue**: Integration tests fail on macOS with errors related to real NVMe devices.

**Solution**:
- Use the `--mock` flag with integration tests:
  ```bash
  ./build/bin/integration_tests --mock
  ```
- Or, use Docker for running integration tests with real SPDK.

## Docker-Specific Issues

### Container Cannot Access NVMe Devices

**Issue**: The Docker container cannot access NVMe devices.

**Solutions**:
- Run the container with the `--privileged` flag:
  ```bash
  docker run --privileged nvmeof-benchmarking
  ```
- Mount the NVMe device nodes into the container:
  ```bash
  docker run -v /dev/nvme0:/dev/nvme0 nvmeof-benchmarking
  ```

### Docker Build Fails

**Issue**: The Docker build process fails.

**Solutions**:
- Update Docker to the latest version.
- Check the Dockerfile for errors.
- Increase Docker resource limits in Docker Desktop settings.

### Docker Performance Issues

**Issue**: Performance in Docker is significantly worse than native execution.

**Solutions**:
- Use the `--net=host` flag for networking benchmarks:
  ```bash
  docker run --net=host nvmeof-benchmarking
  ```
- Use the Docker host network for better performance:
  ```yaml
  # In docker-compose.yml
  services:
    nvmeof-benchmarking:
      network_mode: "host"
  ```
- For macOS, be aware that Docker Desktop uses virtualization which adds overhead.

## Still Having Issues?

If you are still experiencing issues after trying these troubleshooting steps, please open an issue on our [GitHub Issue Tracker](https://github.com/muditbhargava66/nvmeof-benchmark-suite/issues) with detailed information about your problem, including:

- Detailed error messages
- System information (OS, kernel version, etc.)
- Steps to reproduce the issue
- Logs and relevant output

Our team will help you resolve the issue as quickly as possible.