#!/bin/bash
#
# Script to run NVMe-oF benchmarks with various workload profiles
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Determine OS
OS=$(uname -s)

# Set the build directory
BUILD_DIR="$PROJECT_ROOT/build"

# Set the output directory for benchmark results
OUTPUT_DIR="$PROJECT_ROOT/data/benchmark_results"
mkdir -p "$OUTPUT_DIR"

# Set the workload profiles directory
WORKLOAD_PROFILES_DIR="$PROJECT_ROOT/data/workload_profiles"

# Parse command line arguments
PROFILE=""
MONITOR=false
OPTIMIZE=false
VISUALIZE=false

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -p, --profile PROFILE  Specify a single workload profile to run"
    echo "  -m, --monitor          Enable resource monitoring"
    echo "  -o, --optimize         Enable automatic optimization"
    echo "  -v, --visualize        Visualize results after benchmark"
    echo "  -h, --help             Display this help message"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--profile)
            PROFILE="$2"
            shift 2
            ;;
        -m|--monitor)
            MONITOR=true
            shift
            ;;
        -o|--optimize)
            OPTIMIZE=true
            shift
            ;;
        -v|--visualize)
            VISUALIZE=true
            shift
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo "Error: Unknown option $1"
            print_usage
            exit 1
            ;;
    esac
done

# Check if the benchmark executable exists
BENCHMARK_EXE="$BUILD_DIR/bin/nvmeof_benchmarking"
if [ ! -f "$BENCHMARK_EXE" ]; then
    echo "Error: Benchmark executable not found at $BENCHMARK_EXE"
    echo "Please run the build script first."
    exit 1
fi

# Function to run a benchmark with a specific profile
run_benchmark() {
    local profile="$1"
    local profile_name=$(basename "$profile" .json)
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local output_file="$OUTPUT_DIR/${profile_name}_$timestamp.csv"
    
    echo "======================================="
    echo "Running benchmark with profile: $profile_name"
    echo "======================================="
    
    # Build command with flags
    CMD="$BENCHMARK_EXE --workload-profile \"$profile\" --output-dir \"$OUTPUT_DIR\""
    
    if [ "$MONITOR" = true ]; then
        CMD="$CMD --monitor"
    fi
    
    if [ "$OPTIMIZE" = true ]; then
        CMD="$CMD --optimize"
    fi
    
    if [ "$VISUALIZE" = true ]; then
        CMD="$CMD --visualize"
    fi
    
    # Run the benchmark
    eval "$CMD"
    
    if [ $? -eq 0 ]; then
        echo "Benchmark completed successfully."
        echo "Results saved to: $output_file"
    else
        echo "Benchmark failed."
        return 1
    fi
    
    echo "---------------------------------------"
}

# Run benchmarks
if [ -n "$PROFILE" ]; then
    # Run a specific profile
    if [ -f "$PROFILE" ]; then
        run_benchmark "$PROFILE"
    elif [ -f "$WORKLOAD_PROFILES_DIR/$PROFILE.json" ]; then
        run_benchmark "$WORKLOAD_PROFILES_DIR/$PROFILE.json"
    else
        echo "Error: Profile not found: $PROFILE"
        exit 1
    fi
else
    # Run all profiles in the directory
    PROFILES_FOUND=false
    
    for profile in "$WORKLOAD_PROFILES_DIR"/*.json; do
        if [ -f "$profile" ]; then
            PROFILES_FOUND=true
            run_benchmark "$profile"
        fi
    done
    
    if [ "$PROFILES_FOUND" = false ]; then
        echo "Error: No workload profiles found in $WORKLOAD_PROFILES_DIR"
        exit 1
    fi
fi

echo "All benchmarks completed."
