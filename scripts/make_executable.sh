#!/bin/bash
#
# Script to make all scripts executable
#

set -e

# Determine script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Make all scripts executable
chmod +x "$SCRIPT_DIR"/*.sh

echo "All scripts made executable."
