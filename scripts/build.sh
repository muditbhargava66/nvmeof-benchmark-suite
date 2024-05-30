#!/bin/bash

# Set the project root directory
PROJECT_ROOT=$(dirname "$(dirname "$(readlink -f "$0")")")

# Create a build directory if it doesn't exist
BUILD_DIR="$PROJECT_ROOT/build"
mkdir -p "$BUILD_DIR"

# Change to the build directory
cd "$BUILD_DIR"

# Generate the build files using CMake
cmake "$PROJECT_ROOT"

# Build the project
make

# Check if the build was successful
if [ $? -eq 0 ]; then
    echo "Build completed successfully."
else
    echo "Build failed."
    exit 1
fi