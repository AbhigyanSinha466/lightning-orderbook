# Lightning Orderbook - Working Documentation

## 1. Project Overview
Lightning Orderbook is a high-performance, single-threaded matching engine written in C++17. It is designed to process NASDAQ ITCH 5.0 binary data feeds and execute trades with minimal latency.

## 2. Folder Structure and File Descriptions

### Core Engine (`include/` and `src/`)
These files form the `engine_core` static library.
- **`types.hpp`**: Defines global type aliases (Price, Qty, OrderId), enumerations (Side, OrderType), and constants (PRICE_SCALE).
- **`order.hpp` / `order.cpp`**: The `Order` struct. Holds order data and a cached iterator (`level_it`) for O(1) cancellations.
- **`price_level.hpp` / `price_level.cpp`**: Manages a queue of orders at a specific price point using a doubly-linked list.
- **`order_book.hpp` / `order_book.cpp`**: Represents the bid and ask sides for a single symbol using `std::map`.
- **`matching_engine.hpp` / `matching_engine.cpp`**: The top-level coordinator. Manages multiple `OrderBook` instances, owns `Order` objects via `std::unique_ptr`, and implements the matching logic.

### ITCH Parser (`itch/`)
This module forms the `itch_parser` static library.
- **`itch_parser.hpp` / `itch_parser.cpp`**: A callback-based binary parser for NASDAQ ITCH 5.0. Handles big-endian to host-endian conversion and skip logic for unsupported messages.

### Benchmark Harness (`bench/`)
The `bench` executable for performance measurement.
- **`latency_stats.hpp` / `latency_stats.cpp`**: Collects nanosecond-precision samples and computes percentiles (P50, P99, etc.).
- **`replay_harness.hpp` / `replay_harness.cpp`**: Bridges the parser and engine, measuring the time taken for each engine call.
- **`main_bench.cpp`**: Entry point. Memory-maps the input ITCH file and triggers the replay.

### Utilities (`include/utils/` and `src/utils/`)
- **`logger.hpp` / `logger.cpp`**: A simple, non-blocking diagnostic logger.
- **`ring_buffer.hpp` / `ring_buffer.cpp`**: A fixed-capacity FIFO queue (currently a wrapper around `std::queue`).

### Tests (`tests/`)
- **`test_*.cpp`**: Unit tests for each module using the Catch2 framework.

---

## 3. Compilation Instructions

### Prerequisites
- C++17 compatible compiler (GCC 9+, Clang 10+, or AppleClang)
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

### Running the Benchmark
To replay an ITCH binary file and see performance stats:
```bash
./build/bench <path_to_itch_file>
```

### Cleaning Build Artifacts
To remove compiled binaries and object files while keeping the configuration:
```bash
cmake --build build --target clean
```
Alternatively, if you want to start from scratch, simply remove the `build` directory:
```bash
rm -rf build
```

---

## 4. Architectural Decisions
- **Fixed-Point Arithmetic**: Prices are stored as `int64_t` with 6 decimal places to avoid floating-point jitter.
- **Memory Management**: `MatchingEngine` uses `std::unique_ptr` in an `unordered_map` for clear ownership and O(1) lookup.
- **No Exceptions**: The hot matching path uses assertions rather than exceptions to maintain deterministic performance.
- **O(1) Cancel**: Achieved by storing an iterator to the order's position in the `std::list` directly within the `Order` object.

## 5. Known Limitations
- **Stateless Parser**: The `ItchParser` does not have an internal buffer to accumulate partial messages across calls. It only parses complete messages within the provided buffer and returns the number of bytes consumed. If a buffer ends with a partial message, the caller must manage those remaining bytes and prepend them to the next data chunk.
