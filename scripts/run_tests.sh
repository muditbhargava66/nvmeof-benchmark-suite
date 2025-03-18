#!/bin/bash
#
# Script to run tests for the NVMe-oF Benchmarking Suite
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Determine OS
OS=$(uname -s)

# Set the build directory
BUILD_DIR="$PROJECT_ROOT/build"

# Parse command line arguments
RUN_UNIT_TESTS=true
RUN_INTEGRATION_TESTS=true
VERBOSE=false

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --unit-only        Run only unit tests"
    echo "  --integration-only Run only integration tests"
    echo "  -v, --verbose      Enable verbose output"
    echo "  -h, --help         Display this help message"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --unit-only)
            RUN_UNIT_TESTS=true
            RUN_INTEGRATION_TESTS=false
            shift
            ;;
        --integration-only)
            RUN_UNIT_TESTS=false
            RUN_INTEGRATION_TESTS=true
            shift
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

# Print banner
echo "======================================="
echo "NVMe-oF Benchmarking Suite Tests"
echo "======================================="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found at $BUILD_DIR"
    echo "Please run the build script first."
    exit 1
fi

# Function to run unit tests
run_unit_tests() {
    echo "Running unit tests..."
    
    UNIT_TESTS_EXE="$BUILD_DIR/bin/unit_tests"
    
    if [ ! -f "$UNIT_TESTS_EXE" ]; then
        echo "Error: Unit tests executable not found at $UNIT_TESTS_EXE"
        return 1
    fi
    
    # Run the tests
    if [ "$VERBOSE" = true ]; then
        "$UNIT_TESTS_EXE" --gtest_color=yes
    else
        "$UNIT_TESTS_EXE" --gtest_color=yes --gtest_brief=1
    fi
    
    if [ $? -eq 0 ]; then
        echo "Unit tests passed successfully."
    else
        echo "Some unit tests failed."
        return 1
    fi
}

# Function to run integration tests
run_integration_tests() {
    echo "Running integration tests..."
    
    INTEGRATION_TESTS_EXE="$BUILD_DIR/bin/integration_tests"
    
    if [ ! -f "$INTEGRATION_TESTS_EXE" ]; then
        echo "Error: Integration tests executable not found at $INTEGRATION_TESTS_EXE"
        return 1
    fi
    
    # Run the tests
    if [ "$VERBOSE" = true ]; then
        "$INTEGRATION_TESTS_EXE" --gtest_color=yes
    else
        "$INTEGRATION_TESTS_EXE" --gtest_color=yes --gtest_brief=1
    fi
    
    if [ $? -eq 0 ]; then
        echo "Integration tests passed successfully."
    else
        echo "Some integration tests failed."
        return 1
    fi
}

# Run tests
EXIT_CODE=0

if [ "$RUN_UNIT_TESTS" = true ]; then
    echo "======================================="
    echo "Running Unit Tests"
    echo "======================================="
    run_unit_tests || EXIT_CODE=1
    echo ""
fi

if [ "$RUN_INTEGRATION_TESTS" = true ]; then
    echo "======================================="
    echo "Running Integration Tests"
    echo "======================================="
    run_integration_tests || EXIT_CODE=1
    echo ""
fi

if [ $EXIT_CODE -eq 0 ]; then
    echo "All tests passed successfully."
else
    echo "Some tests failed."
fi

exit $EXIT_CODE
