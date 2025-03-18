# Docker Setup for NVMe-oF Benchmarking Suite

This directory contains Docker configurations for development and production environments for the NVMe-oF Benchmarking Suite.

## Docker Files

- `Dockerfile`: Production image for running benchmarks
- `Dockerfile.dev`: Development image with additional tools for development
- `docker-compose.yml`: Docker Compose configuration for both environments

## Quick Start

From the project root directory, use the provided script to manage Docker:

```bash
# Build development environment
./scripts/docker_run.sh build

# Run development container
./scripts/docker_run.sh run

# Start a bash shell in the development container
./scripts/docker_run.sh bash

# Build production environment
./scripts/docker_run.sh --env prod build

# Run benchmark in production container
./scripts/docker_run.sh --env prod benchmark
```

## Development Environment

The development environment (`Dockerfile.dev`) includes:

- All necessary build tools and dependencies
- Debugging tools (gdb, valgrind)
- Text editors (vim, nano)
- System monitoring tools (htop)
- Pre-built SPDK libraries

This environment is designed for developing and testing the benchmarking suite, with the project directory mounted as a volume for real-time code changes.

## Production Environment

The production environment (`Dockerfile`) is optimized for running benchmarks:

- Multi-stage build for smaller image size
- Minimal runtime dependencies
- Pre-built SPDK libraries
- Ready-to-run benchmarking executable

This environment is designed for running benchmarks in production scenarios, with data and results directories mounted as volumes.

## Usage Examples

### Development Workflow

1. Build the development image:
   ```bash
   ./scripts/docker_run.sh build
   ```

2. Start the development container:
   ```bash
   ./scripts/docker_run.sh run
   ```

3. Enter the container:
   ```bash
   ./scripts/docker_run.sh bash
   ```

4. Inside the container, build the project:
   ```bash
   cd /app/build
   cmake ..
   make
   ```

5. Run tests:
   ```bash
   make test
   ```

### Running Benchmarks

1. Build the production image:
   ```bash
   ./scripts/docker_run.sh --env prod build
   ```

2. Run a benchmark:
   ```bash
   ./scripts/docker_run.sh --env prod benchmark --args "--workload-profile /app/data/workload_profiles/workload_profile_1.json"
   ```

## Configuration

You can customize the Docker setup by modifying:

- `docker-compose.yml`: Container configuration, volumes, environment variables
- `Dockerfile`: Production build process and dependencies
- `Dockerfile.dev`: Development environment tools and dependencies

## Notes for macOS Users

Due to limitations in Docker Desktop for Mac, NVMe device access is not possible. For macOS users, the Docker environment is primarily useful for development and testing without hardware access. For actual NVMe-oF benchmarking, a Linux host is required.

## Troubleshooting

If you encounter issues with Docker, check:

1. Docker daemon is running
2. You have sufficient permissions
3. The project path is correctly mounted
4. SPDK is properly built within the container

For more help, see the [Troubleshooting](../docs/TROUBLESHOOTING.md) guide.