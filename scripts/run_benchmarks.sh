#!/bin/bash

# Set the project root directory
PROJECT_ROOT=$(dirname "$(dirname "$(readlink -f "$0")")")

# Set the build directory
BUILD_DIR="$PROJECT_ROOT/build"

# Set the output directory for benchmark results
OUTPUT_DIR="$PROJECT_ROOT/data/benchmark_results"
mkdir -p "$OUTPUT_DIR"

# Set the workload profiles directory
WORKLOAD_PROFILES_DIR="$PROJECT_ROOT/data/workload_profiles"

# Run benchmarks for each workload profile
for profile in "$WORKLOAD_PROFILES_DIR"/*.json; do
    if [ -e "$profile" ]; then
        echo "Running benchmark with workload profile: $profile"
        "$BUILD_DIR/nvmeof_benchmarking" --workload-profile "$profile" --output-dir "$OUTPUT_DIR"
    fi
done

echo "Benchmarks completed."