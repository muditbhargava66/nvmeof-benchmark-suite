#!/bin/bash
#
# Script to clean up the NVMe-oF Benchmarking Suite project
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Parse command line arguments
CLEAN_BUILD=true
CLEAN_RESULTS=false
CLEAN_ALL=false

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --no-build        Don't clean build directory"
    echo "  --results         Also clean benchmark results"
    echo "  --all             Clean everything (build, results, and generated files)"
    echo "  -h, --help        Display this help message"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-build)
            CLEAN_BUILD=false
            shift
            ;;
        --results)
            CLEAN_RESULTS=true
            shift
            ;;
        --all)
            CLEAN_ALL=true
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

# Print banner
echo "======================================="
echo "NVMe-oF Benchmarking Suite Cleanup"
echo "======================================="

# Clean build directory
if [ "$CLEAN_BUILD" = true ] || [ "$CLEAN_ALL" = true ]; then
    echo "Cleaning build directory..."
    if [ -d "$PROJECT_ROOT/build" ]; then
        rm -rf "$PROJECT_ROOT/build"
        echo "  Build directory removed."
    else
        echo "  Build directory doesn't exist."
    fi
fi

# Clean results
if [ "$CLEAN_RESULTS" = true ] || [ "$CLEAN_ALL" = true ]; then
    echo "Cleaning benchmark results..."
    if [ -d "$PROJECT_ROOT/data/benchmark_results" ]; then
        rm -rf "$PROJECT_ROOT/data/benchmark_results"/*
        echo "  Benchmark results removed."
    else
        echo "  Benchmark results directory doesn't exist."
    fi
    
    echo "Cleaning analysis reports..."
    if [ -d "$PROJECT_ROOT/data/analysis_reports" ]; then
        rm -rf "$PROJECT_ROOT/data/analysis_reports"/*
        echo "  Analysis reports removed."
    else
        echo "  Analysis reports directory doesn't exist."
    fi
fi

# Clean everything
if [ "$CLEAN_ALL" = true ]; then
    echo "Cleaning all generated files..."
    
    # Remove generated workload profiles
    if [ -d "$PROJECT_ROOT/data/workload_profiles" ]; then
        rm -rf "$PROJECT_ROOT/data/workload_profiles"/*
        echo "  Workload profiles removed."
    fi
    
    # Remove generated optimization configs
    if [ -d "$PROJECT_ROOT/data/optimization_configs" ]; then
        rm -rf "$PROJECT_ROOT/data/optimization_configs"/*
        echo "  Optimization configs removed."
    fi
fi

echo "Cleanup completed."
