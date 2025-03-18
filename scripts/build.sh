#!/bin/bash
#
# Script to build the NVMe-oF Benchmarking Suite
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Determine OS
OS=$(uname -s)

# Determine number of CPU cores for parallel build
if [ "$OS" == "Linux" ]; then
    NUM_CORES=$(nproc)
elif [ "$OS" == "Darwin" ]; then
    NUM_CORES=$(sysctl -n hw.ncpu)
else
    NUM_CORES=2  # Default fallback
fi

# Parse command line arguments
BUILD_TYPE="Release"
BUILD_TESTS=true
CLEAN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --no-tests)
            BUILD_TESTS=false
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --debug      Build with debug configuration"
            echo "  --release    Build with release configuration (default)"
            echo "  --no-tests   Skip building tests"
            echo "  --clean      Clean build directory before building"
            echo "  --help       Display this help message"
            exit 0
            ;;
        *)
            echo "Error: Unknown option $1"
            echo "Run '$0 --help' for usage information."
            exit 1
            ;;
    esac
done

# Create a build directory if it doesn't exist
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# Clean build directory if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"/*
fi

# Change to the build directory
cd "$BUILD_DIR"

echo "Configuring CMake build..."
CMAKE_ARGS=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")

if [ "$BUILD_TESTS" = false ]; then
    CMAKE_ARGS+=("-DBUILD_TESTS=OFF")
fi

cmake "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"

echo "Building project with $NUM_CORES cores..."
cmake --build . -- -j "$NUM_CORES"

# Check if the build was successful
if [ $? -eq 0 ]; then
    echo "Build completed successfully."
    echo "Executables can be found in: $BUILD_DIR/bin"
else
    echo "Build failed."
    exit 1
fi
