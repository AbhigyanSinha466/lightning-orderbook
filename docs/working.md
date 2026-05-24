# Lightning Orderbook - Working Documentation

## 1. Project Overview
Lightning Orderbook is a high-performance, single-threaded matching engine written in C++20. It is designed to process NASDAQ ITCH 5.0 binary data feeds and execute trades with minimal latency.

To explore performance tradeoffs, the engine features two distinct structural implementations for the order book's price levels: a **Linked List Implementation (`list_impl`)** and a **Vector Implementation (`vector_impl`)**.

## 2. Folder Structure and File Descriptions

### Engine Implementations
The core engine logic is split into two separate directories, which build into `engine_core_list` and `engine_core_vector` static libraries.

#### 1. List Implementation (`list_impl/`)
Traditional approach using a doubly-linked list for the queue of orders at a specific price point.
- Provides O(1) cancellations since the `Order` struct can cache an iterator to its position.
- Uses `std::map` for the order book bid/ask sides.

#### 2. Vector Implementation (`vector_impl/`)
Alternative approach using contiguous memory (e.g., `std::vector`) for the queue of orders.
- Optimizes for cache locality and fast iteration.
- Trade-offs involve shift operations during cancellations vs. cache misses during list traversal.

#### Core Files in Both Implementations (`include/` & `src/`)
- **`types.hpp`**: Defines global type aliases (Price, Qty, OrderId), enumerations (Side, OrderType), and constants (PRICE_SCALE).
- **`order.hpp` / `order.cpp`**: The `Order` struct. Holds order data.
- **`price_level.hpp` / `price_level.cpp`**: Manages the queue of orders at a specific price point.
- **`order_book.hpp` / `order_book.cpp`**: Represents the bid and ask sides for a single symbol.
- **`matching_engine.hpp` / `matching_engine.cpp`**: The top-level coordinator managing multiple `OrderBook` instances.
- **`utils/logger.hpp` & `utils/ring_buffer.hpp`**: Diagnostic logger and FIFO queue.

### ITCH Parser (`itch/`)
This module forms the `itch_parser` static library. It is implementation-agnostic and links with either engine backend.
- **`itch_parser.hpp` / `itch_parser.cpp`**: A callback-based binary parser for NASDAQ ITCH 5.0. Handles big-endian to host-endian conversion and skip logic for unsupported messages.

### Benchmark Harness (`bench/`)
The source for high-precision performance measurement. The build system creates two executables: `bench_list` and `bench_vector`.
- **`cycle_clock.hpp` / `cycle_clock.cpp`**: Interfaces with hardware performance counters (using `kperf` on Apple Silicon) to measure exact CPU clock cycles spent on code execution. Includes a runtime self-calibration routine to calculate frequency (GHz).
- **`latency_stats.hpp` / `latency_stats.cpp`**: Collects cycle-count samples and computes percentiles. Dumps data in both raw cycles and calibrated nanoseconds.
- **`replay_harness.hpp` / `replay_harness.cpp`**: Bridges the parser and engine, measuring the precise cycle delta for each message processed.
- **`main_bench.cpp`**: Entry point. Calibrates the clock, memory-maps the ITCH file, and triggers the replay. Requires `sudo` on macOS for hardware counter access.

### Tests (`tests/`)
- **`test_*.cpp`**: Unit tests for the modules using Catch2. (Currently links with `engine_core_list` by default in CMake).

### Scripts (`scripts/`)
- Python and Bash scripts (`compare_impls.py`, `run_comparison.sh`, `plot_latency.py`) used to run benchmarks against both implementations and visualize the performance differences.

---

## 3. Compilation Instructions

### Prerequisites
- C++20 compatible compiler
- CMake 3.15 or higher
- Internet connection (to fetch Catch2 during first build)

### Build Steps
1. **Configure CMake:**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```
2. **Build everything:**
   ```bash
   cmake --build build
   ```

### Running Tests
After building, execute the test suite:
```bash
./build/tests
```

### Running the Benchmarks
To replay an ITCH binary file and see performance stats, run either executable with `sudo` (required for hardware counters):
```bash
# Test the list implementation
sudo ./build/bench_list data/synthetic.itch

# Test the vector implementation
sudo ./build/bench_vector data/synthetic.itch
```
Alternatively, use the automated script which handles building, data generation, and plotting:
```bash
./scripts/run_comparison.sh
```
The comparison script will generate high-resolution plots (`comparison_results.png`) with 300 bins, locked to a 1500ns x-axis for consistent visual analysis.

### Cleaning Build Artifacts
```bash
rm -rf build
```

---

## 4. Architectural Decisions
- **Implementation Split**: The dual-implementation architecture (`list_impl` vs `vector_impl`) allows empirical measurement of cache locality vs algorithmic complexity (O(1) list cancel vs contiguous memory).
- **Fixed-Point Arithmetic**: Prices are stored as `int64_t` with 6 decimal places to avoid floating-point jitter.
- **No Exceptions**: The hot matching path uses assertions rather than exceptions to maintain deterministic performance.

## 5. Known Limitations
- **Stateless Parser**: The `ItchParser` does not have an internal buffer to accumulate partial messages across calls.
- **Test Coverage**: The Catch2 test suite currently focuses on `engine_core_list`.

## 6. Git Configuration
- **Ignored Files**: We have configured `.gitignore` to exclude all `.csv` files. This ensures that large benchmark output data or generated test data files are not accidentally committed to the repository.