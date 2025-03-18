#!/bin/bash
#
# Script to set up the NVMe-oF Benchmarking Suite development environment
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Determine OS
OS=$(uname -s)

# Print banner
echo "======================================="
echo "NVMe-oF Benchmarking Suite Setup"
echo "======================================="

# Check if required tools are installed
echo "Checking prerequisites..."
MISSING_TOOLS=()

# Required tools
TOOLS=("git" "cmake" "g++" "make")

for tool in "${TOOLS[@]}"; do
    if ! command -v "$tool" &> /dev/null; then
        MISSING_TOOLS+=("$tool")
    fi
done

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
    echo "Error: Missing required tools: ${MISSING_TOOLS[*]}"
    
    if [ "$OS" == "Linux" ]; then
        echo "Please install them using your package manager."
        echo "For Ubuntu/Debian: sudo apt-get install ${MISSING_TOOLS[*]}"
        echo "For CentOS/RHEL: sudo yum install ${MISSING_TOOLS[*]}"
    elif [ "$OS" == "Darwin" ]; then
        echo "Please install them using Homebrew:"
        echo "brew install ${MISSING_TOOLS[*]}"
    fi
    
    exit 1
fi

echo "All prerequisites are installed."

# Create required directories
echo "Creating project directories..."
mkdir -p "$PROJECT_ROOT/data/workload_profiles"
mkdir -p "$PROJECT_ROOT/data/benchmark_results"
mkdir -p "$PROJECT_ROOT/data/analysis_reports"
mkdir -p "$PROJECT_ROOT/data/optimization_configs"

# Install/setup SPDK
echo "Setting up SPDK..."
"$SCRIPT_DIR/install_spdk.sh"

# Generate sample workload profiles if they don't exist
SAMPLE_PROFILE="$PROJECT_ROOT/data/workload_profiles/workload_profile_1.json"
if [ ! -f "$SAMPLE_PROFILE" ]; then
    echo "Generating sample workload profiles..."
    
    cat > "$SAMPLE_PROFILE" << EOF
{
  "name": "Sequential Read",
  "description": "100% sequential read workload",
  "total_size": 1073741824,
  "block_size": 4096,
  "num_blocks": 262144,
  "read_percentage": 100,
  "write_percentage": 0,
  "random_percentage": 0
}
EOF

    cat > "$PROJECT_ROOT/data/workload_profiles/workload_profile_2.json" << EOF
{
  "name": "Random Read/Write",
  "description": "70% read, 30% write, 100% random workload",
  "total_size": 1073741824,
  "block_size": 4096,
  "num_blocks": 262144,
  "read_percentage": 70,
  "write_percentage": 30,
  "random_percentage": 100
}
EOF

    cat > "$PROJECT_ROOT/data/workload_profiles/workload_profile_3.json" << EOF
{
  "name": "Sequential Write",
  "description": "100% sequential write workload",
  "total_size": 1073741824,
  "block_size": 4096,
  "num_blocks": 262144,
  "read_percentage": 0,
  "write_percentage": 100,
  "random_percentage": 0
}
EOF
fi

# Generate sample optimization config if it doesn't exist
SAMPLE_CONFIG="$PROJECT_ROOT/data/optimization_configs/optimization_config_1.json"
if [ ! -f "$SAMPLE_CONFIG" ]; then
    echo "Generating sample optimization configs..."
    
    cat > "$SAMPLE_CONFIG" << EOF
{
  "cpu_bottleneck": "cpu_governor=performance",
  "memory_bottleneck": "vm.swappiness=10",
  "network_bottleneck": "net.core.rmem_max=16777216,net.core.wmem_max=16777216"
}
EOF
fi

# Build the project
echo "Building the project..."
"$SCRIPT_DIR/build.sh"

echo "======================================="
echo "Setup completed successfully!"
echo "======================================="
echo ""
echo "Next steps:"
echo "  1. Run benchmarks: ./scripts/run_benchmarks.sh"
echo "  2. Analyze results: ./scripts/analyze_results.sh"
echo "  3. Visualize results: ./scripts/visualize_results.sh"
echo ""
echo "For more information, see the documentation."
