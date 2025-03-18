#!/bin/bash
#
# Script to visualize benchmark results
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Set the benchmark results directory
RESULTS_DIR="$PROJECT_ROOT/data/benchmark_results"

# Parse command line arguments
RESULT_FILE=""

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -f, --file FILE    Visualize a specific result file"
    echo "  -h, --help         Display this help message"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -f|--file)
            RESULT_FILE="$2"
            shift 2
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

# Check if the visualizer executable exists
VISUALIZER_EXE="$PROJECT_ROOT/build/bin/nvmeof_visualizer"
if [ ! -f "$VISUALIZER_EXE" ]; then
    echo "Error: Visualizer executable not found at $VISUALIZER_EXE"
    echo "Please run the build script first."
    exit 1
fi

# Function to visualize a specific result file
visualize_file() {
    local file="$1"
    local filename=$(basename "$file")
    
    echo "======================================="
    echo "Visualizing: $filename"
    echo "======================================="
    
    # Run the visualizer
    "$VISUALIZER_EXE" --input-file "$file"
    
    if [ $? -eq 0 ]; then
        echo "Visualization completed successfully."
    else
        echo "Visualization failed."
        return 1
    fi
    
    echo "---------------------------------------"
}

# Run visualization
if [ -n "$RESULT_FILE" ]; then
    # Visualize a specific file
    if [ -f "$RESULT_FILE" ]; then
        visualize_file "$RESULT_FILE"
    else
        echo "Error: Result file not found: $RESULT_FILE"
        exit 1
    fi
else
    # Visualize the most recent result file
    latest_file=$(ls -t "$RESULTS_DIR"/*.csv 2>/dev/null | head -n 1)
    
    if [ -n "$latest_file" ]; then
        echo "Using most recent result file: $(basename "$latest_file")"
        visualize_file "$latest_file"
    else
        echo "Error: No result files found in $RESULTS_DIR"
        exit 1
    fi
fi

echo "Visualization completed."
