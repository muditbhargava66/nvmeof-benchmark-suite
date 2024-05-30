#!/bin/bash

# Set the project root directory
PROJECT_ROOT=$(dirname "$(dirname "$(readlink -f "$0")")")

# Set the benchmark results directory
RESULTS_DIR="$PROJECT_ROOT/data/benchmark_results"

# Set the output directory for analysis reports
ANALYSIS_DIR="$PROJECT_ROOT/data/analysis_reports"
mkdir -p "$ANALYSIS_DIR"

# Analyze benchmark results
"$PROJECT_ROOT/build/nvmeof_analysis" --results-dir "$RESULTS_DIR" --output-dir "$ANALYSIS_DIR"

# Check if the analysis was successful
if [ $? -eq 0 ]; then
    echo "Analysis completed successfully."
    echo "Reports generated in: $ANALYSIS_DIR"
else
    echo "Analysis failed."
    exit 1
fi