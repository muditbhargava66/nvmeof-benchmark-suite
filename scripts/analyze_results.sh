#!/bin/bash
#
# Script to analyze benchmark results
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Set the benchmark results directory
RESULTS_DIR="$PROJECT_ROOT/data/benchmark_results"

# Set the output directory for analysis reports
ANALYSIS_DIR="$PROJECT_ROOT/data/analysis_reports"
mkdir -p "$ANALYSIS_DIR"

# Parse command line arguments
RESULT_FILE=""
VERBOSE=false

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -f, --file FILE    Analyze a specific result file"
    echo "  -v, --verbose      Enable verbose output"
    echo "  -h, --help         Display this help message"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -f|--file)
            RESULT_FILE="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
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

# Check if the analysis executable exists
ANALYSIS_EXE="$PROJECT_ROOT/build/bin/nvmeof_analysis"
if [ ! -f "$ANALYSIS_EXE" ]; then
    echo "Error: Analysis executable not found at $ANALYSIS_EXE"
    echo "Please run the build script first."
    exit 1
fi

# Function to analyze a specific result file
analyze_file() {
    local file="$1"
    local filename=$(basename "$file")
    local output_file="$ANALYSIS_DIR/${filename%.csv}_analysis.txt"
    
    echo "Analyzing: $filename"
    
    # Build command
    CMD="$ANALYSIS_EXE --results-file \"$file\" --output-dir \"$ANALYSIS_DIR\""
    
    if [ "$VERBOSE" = true ]; then
        CMD="$CMD --verbose"
    fi
    
    # Run the analysis
    eval "$CMD"
    
    if [ $? -eq 0 ]; then
        echo "Analysis completed successfully."
        echo "Report saved to: $output_file"
    else
        echo "Analysis failed."
        return 1
    fi
}

# Run analysis
if [ -n "$RESULT_FILE" ]; then
    # Analyze a specific file
    if [ -f "$RESULT_FILE" ]; then
        analyze_file "$RESULT_FILE"
    else
        echo "Error: Result file not found: $RESULT_FILE"
        exit 1
    fi
else
    # Analyze all CSV files in the results directory
    FILES_FOUND=false
    
    for file in "$RESULTS_DIR"/*.csv; do
        if [ -f "$file" ]; then
            FILES_FOUND=true
            analyze_file "$file"
        fi
    done
    
    if [ "$FILES_FOUND" = false ]; then
        echo "Error: No result files found in $RESULTS_DIR"
        exit 1
    fi
fi

echo "All analyses completed."
echo "Reports are available in: $ANALYSIS_DIR"
