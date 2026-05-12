#!/bin/bash
set -e

# 1. Setup data
echo "Checking for synthetic data..."
if [ ! -f "data/synthetic.itch" ]; then
    echo "Generating synthetic data..."
    python3 scripts/generate_itch.py
fi

# 2. Build both implementations
echo "Building benchmarks..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target bench_list bench_vector

# 3. Run benchmarks
echo "Running List-based benchmark (requires sudo for Apple Silicon performance counters)..."
sudo ./build/bench_list data/synthetic.itch latency_list.csv

echo "Running Vector-based benchmark (requires sudo for Apple Silicon performance counters)..."
sudo ./build/bench_vector data/synthetic.itch latency_vector.csv

# 4. Compare results
echo "Generating comparison plot..."
python3 scripts/compare_impls.py latency_list.csv latency_vector.csv
