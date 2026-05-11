#!/bin/bash

# Exit on any error
set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building the project...${NC}"
cmake --build build

echo -e "\n${GREEN}Running unit tests...${NC}"
./build/tests

# Check if a benchmark file was provided as an argument
if [ ! -z "$1" ]; then
    if [ -f "$1" ]; then
        echo -e "\n${GREEN}Running benchmarks with file: $1...${NC}"
        ./build/bench "$1"
    else
        echo -e "\n${RED}Error: Benchmark file '$1' not found.${NC}"
        exit 1
    fi
fi

echo -e "\n${GREEN}All checks passed successfully!${NC}"
