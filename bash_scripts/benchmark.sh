#!/bin/bash

# Exit on any error
set -e

# Configuration
DATA_DIR="data"
DEFAULT_ITCH_FILE="$DATA_DIR/synthetic.itch"
NUM_MESSAGES=1000000

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 1. Build the project (Release mode for accurate benchmarks)
echo -e "${GREEN}Building project in Release mode...${NC}"
cmake -B build -DCMAKE_BUILD_TYPE=Release
sudo cmake --build build --target bench

# 2. Check for ITCH data file
if [ ! -f "$DEFAULT_ITCH_FILE" ]; then
    echo -e "${YELLOW}ITCH data file not found at $DEFAULT_ITCH_FILE. Generating synthetic data...${NC}"
    mkdir -p "$DATA_DIR"
    # Note: Using python3 as it's common on macOS/Linux
    python3 scripts/generate_itch.py
    # The script generates to ../data if run from scripts/, but we run from root.
    # We might need to adjust the path or just run it and move if it puts it in the wrong place.
    # Actually, generate_itch.py uses data_dir = "../data" relative to itself.
    # Let's just run it from scripts/ to be safe.
    pushd scripts > /dev/null
    python3 generate_itch.py
    popd > /dev/null
fi

# 3. Run the benchmark
echo -e "\n${GREEN}Starting benchmark...${NC}"
sudo ./build/bench "$DEFAULT_ITCH_FILE"

# 4. Generate the plot
if [ -f "latency_results.csv" ]; then
    echo -e "\n${GREEN}Generating latency distribution plot...${NC}"
    # Check if matplotlib and pandas are available
    if python3 -c "import pandas, matplotlib" &> /dev/null; then
        python3 scripts/plot_latency.py latency_results.csv
    else
        echo -e "${YELLOW}Warning: Python dependencies (pandas, matplotlib) not found. Skipping plot generation.${NC}"
        echo -e "To install them, run: pip install pandas matplotlib"
    fi
else
    echo -e "${RED}Error: latency_results.csv was not generated.${NC}"
fi

echo -e "\n${GREEN}Benchmark complete!${NC}"
