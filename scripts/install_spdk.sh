#!/bin/bash
#
# Script to install SPDK in the third_party directory
#

set -e

# Determine script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"
THIRD_PARTY_DIR="$PROJECT_ROOT/third_party"
SPDK_DIR="$THIRD_PARTY_DIR/spdk"

# Determine OS
OS=$(uname -s)

# Function to print error message and exit
error_exit() {
    echo "ERROR: $1" >&2
    exit 1
}

# Function to check for required tools
check_prerequisites() {
    local missing_tools=()
    
    # Check for git
    if ! command -v git &> /dev/null; then
        missing_tools+=("git")
    fi
    
    # Check for gcc/g++
    if ! command -v gcc &> /dev/null; then
        missing_tools+=("gcc")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_tools+=("g++")
    fi
    
    # Check for make
    if ! command -v make &> /dev/null; then
        missing_tools+=("make")
    fi
    
    # Check for cmake
    if ! command -v cmake &> /dev/null; then
        missing_tools+=("cmake")
    fi
    
    # If we found missing tools, error out
    if [ ${#missing_tools[@]} -ne 0 ]; then
        error_exit "Missing required tools: ${missing_tools[*]}"
    fi
}

# Function to install SPDK on Linux
install_spdk_linux() {
    echo "Installing SPDK on Linux..."
    
    # Create directory
    mkdir -p "$THIRD_PARTY_DIR"
    
    # Clone SPDK if it doesn't exist
    if [ ! -d "$SPDK_DIR" ]; then
        echo "Cloning SPDK repository..."
        git clone --recurse-submodules https://github.com/spdk/spdk.git "$SPDK_DIR"
    else
        echo "SPDK directory already exists. Updating..."
        cd "$SPDK_DIR"
        git pull
        git submodule update --init --recursive
    fi
    
    # Build SPDK
    cd "$SPDK_DIR"
    echo "Installing SPDK dependencies..."
    ./scripts/pkgdep.sh
    
    echo "Configuring SPDK..."
    ./configure --disable-tests --disable-unit-tests --disable-examples
    
    echo "Building SPDK..."
    make -j$(nproc)
    
    echo "SPDK installation complete."
    echo "SPDK_DIR=$SPDK_DIR"
}

# Function to set up SPDK mock on macOS
setup_spdk_mock_macos() {
    echo "Setting up SPDK mock for macOS development..."
    
    # Create directories
    mkdir -p "$THIRD_PARTY_DIR/spdk_mock"
    mkdir -p "$THIRD_PARTY_DIR/spdk_mock/include/spdk"
    mkdir -p "$THIRD_PARTY_DIR/spdk_mock/src"
    
    # Check if mock files already exist
    if [ -f "$THIRD_PARTY_DIR/spdk_mock/include/spdk/nvme.h" ] && \
       [ -f "$THIRD_PARTY_DIR/spdk_mock/src/nvme.c" ] && \
       [ -f "$THIRD_PARTY_DIR/spdk_mock/CMakeLists.txt" ]; then
        echo "SPDK mock files already exist. Skipping setup."
    else
        echo "Mock files not found. Please copy the mock implementation files manually."
        echo "See the project documentation for instructions."
    fi
    
    echo "SPDK mock setup complete."
}

# Main script execution
echo "NVMe-oF Benchmarking Suite SPDK Installation Script"
echo "==================================================="

# Check prerequisites
check_prerequisites

# Install based on OS
if [ "$OS" == "Linux" ]; then
    install_spdk_linux
elif [ "$OS" == "Darwin" ]; then
    setup_spdk_mock_macos
else
    error_exit "Unsupported operating system: $OS"
fi

echo "Installation completed successfully."
exit 0