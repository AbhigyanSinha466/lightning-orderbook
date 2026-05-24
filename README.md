# Lightning Orderbook

Lightning Orderbook is a high-performance, single-threaded matching engine written in C++20. It is designed to process NASDAQ ITCH 5.0 binary data feeds and execute trades with minimal latency. 

The project explores the performance trade-offs of different data structures and memory management techniques, evolving from a standard library-based approach to a highly optimized implementation using flat maps, intrusive lists, and custom memory pooling.

## 🚀 Key Features

- **High-Performance Matching**: Optimized for sub-microsecond latency.
- **NASDAQ ITCH 5.0 Support**: Includes a high-speed binary parser for ITCH 5.0 feeds.
- **Multiple Implementations**: Compare different architectural approaches to understand performance bottlenecks.
- **Precision Benchmarking**: Hardware performance counter-based cycle-accurate latency measurements.
- **Fast Data Ingestion**: Uses memory-mapped files (`mmap`) for zero-copy access to large binary data feeds.
- **Symbol Hashing**: Symbols are pre-hashed to `uint64_t` for O(1) dictionary lookups and comparison.
- **Deterministic**: Fixed-point arithmetic for prices (4 decimal places per ITCH 5.0) and zero-exception hot paths.

---

## 🏗 Architectural Implementations

The project features three distinct implementations of the order book, each building on the lessons learned from the previous one.

| Feature | `list_impl` | `vector_impl` | `f_vector_impl` |
| :--- | :--- | :--- | :--- |
| **Matching Engine** | `std::map` (RB Tree) | `std::map` (RB Tree) | **`absl::flat_hash_map`** |
| **Price Map** | `std::map` (RB Tree) | `std::vector` (Flat Map) | `std::vector` (Flat Map) |
| **Order Queue** | `std::list` (Standard) | Intrusive Doubly-Linked List | Intrusive Doubly-Linked List |
| **Memory** | `std::allocator` | `std::allocator` | **Custom `OrderPool`** |
| **Cache Optimization**| Minimal | Better | **Best (Alignment + Pool)** |

### 1. List Implementation (`list_impl`)
A baseline approach using standard library containers. It uses a `std::map` to store price levels and a `std::list` for the queue of orders at each price. While providing O(1) cancellations, it suffers from cache misses due to the node-based nature of both `std::map` and `std::list`.

### 2. Vector Implementation (`vector_impl`)
Optimizes for cache locality by replacing the per-book price levels map with a **Flat Map** (a sorted `std::vector` of price entries). It also introduces an **Intrusive Linked List** for the order queue, which embeds the pointers directly within the `Order` struct, reducing the number of allocations and pointer indirections.

### 3. Fast Vector Implementation (`f_vector_impl`)
The most optimized version. It builds on the `vector_impl` but applies further system-wide optimizations:
- **`absl::flat_hash_map`**: Replaces standard maps in the `MatchingEngine` with Abseil's high-performance hash map for faster symbol and order lookups.
- **Custom Order Pool**: Pre-allocates memory for `Order` objects in large blocks (arenas) to eliminate the overhead of `new`/`delete` and reduce heap fragmentation.
- **Cache Alignment**: Uses `alignas(64)` to ensure `Order` and `PriceLevel` objects are aligned to CPU cache lines, preventing false sharing and optimizing fetch performance.

---

## ⏱ Performance Measurement & Profiling

To achieve sub-microsecond latency, standard wall-clock timing is insufficient. This project employs several high-precision measurement techniques and recommends industry-standard tools for deep analysis.

### 1. Cycle-Accurate Measurement (Project Built-in)
The `CycleClock` module provides access to hardware CPU cycle counters, which are the most granular metric available.

- **macOS (Apple Silicon)**: Uses the private `kperf` framework to read ARM's PMU (Performance Monitoring Unit) counters. This requires `sudo` privileges to enable thread-level counting.
- **Linux (Planned/Manual)**: Uses `rdtsc` (x86) or `pmccntr_el0` (ARM64) instructions.

### 2. Linux Performance Tools (`perf`)
On Linux, the `perf` subsystem is the gold standard for profiling.

- **Stat Analysis**: Get a high-level overview of cache misses and branch mispredictions:
  ```bash
  perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./build/bench_f_vector data/synthetic.itch
  ```
- **Record & Report**: Identify hot spots and "hot" assembly instructions:
  ```bash
  perf record -g ./build/bench_f_vector data/synthetic.itch
  perf report
  ```
- **Flame Graphs**: Visualize the call stack to find bottlenecks:
  ```bash
  perf record -g ./build/bench_f_vector data/synthetic.itch
  perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > profile.svg
  ```

### 3. macOS Performance Tools (`Instruments`)
For Apple Silicon, the **Xcode Instruments** suite provides powerful GUI-based analysis.

- **Time Profiler**: Identifies which functions consume the most CPU time.
- **Counters**: Accesses hardware PMU counters for instructions per cycle (IPC), L1/L2 cache misses, and more.
- **System Trace**: Analyzes system calls and scheduling latency.
- **Command Line**: Use `sample` for a quick sampling-based profile:
  ```bash
  sample ./build/bench_f_vector 10
  ```

### 4. Cache & Memory Analysis (`Valgrind`)
To understand memory access patterns and cache efficiency:

- **Cachegrind**: Simulates L1/L2 caches to identify exactly which lines of code cause cache misses.
  ```bash
  valgrind --tool=cachegrind ./build/bench_f_vector data/synthetic.itch
  cg_annotate cachegrind.out.<pid>
  ```
- **Massif**: Profiles heap memory usage over time to ensure the `OrderPool` is behaving as expected.
  ```bash
  valgrind --tool=massif ./build/bench_f_vector data/synthetic.itch
  ms_print massif.out.<pid>
  ```

---

## 🔬 Benchmark Results Visualization

To compare the implementations, use the provided automated script. This will generate a latency distribution plot (`comparison_results.png`) that highlights the performance differences between the `list`, `vector`, and `fast vector` versions.

```bash
# Run the full comparison suite
./scripts/run_comparison.sh
```

### Understanding the Results
- **P50 (Median)**: Represents the typical latency for a message. The `f_vector_impl` shows a massive improvement here (a sharp, left-shifted peak) due to superior cache locality and the avoidance of per-message `new`/`delete` allocations.
- **P90/P99 (Tail Latency)**: Indicates how the engine handles bursts. Counter-intuitively, the `f_vector_impl` can exhibit a **longer tail** (visible as a flat line extending to the right). This is an artifact of the `OrderPool`'s memory management: when the pre-allocated arena exhausts its capacity, it must synchronously allocate a massive new block of memory, causing a significant latency spike for the unlucky message that triggered the expansion. 
- **Distribution Shape**: A narrow, high peak (like the green `f_vector` plot) indicates highly deterministic performance for the vast majority of operations, even if the absolute maximum latency is occasionally higher.

---

## 🪵 High-Performance Logging

In low-latency systems, traditional synchronous logging (e.g., `std::cout` or writing to a file) is a major source of **jitter**. This project uses an asynchronous logging architecture to keep the matching path deterministic.

### 1. Lock-Free Ring Buffer
The core of the logging system is a **Single-Producer Single-Consumer (SPSC) Lock-Free Ring Buffer**.
- **Batching**: Updates to the shared read/write counters are batched to minimize cache-coherency traffic between CPU cores.
- **Cache Alignment**: The producer and consumer counters are placed on separate cache lines (`alignas(64)`) to prevent **false sharing**.
- **Memory Barriers**: Uses `std::memory_order_acquire` and `std::memory_order_release` to ensure thread safety without the heavy overhead of mutexes.

### 2. Asynchronous Logger
The `Logger` offloads the expensive task of string formatting and I/O to a background thread.
- **Hot Path**: The matching engine simply pushes a small, fixed-size event (like a `FillEvent`) onto the Ring Buffer. This is an O(1) operation that takes only a few nanoseconds.
- **Background Thread**: A dedicated consumer thread polls the Ring Buffer, formats the logs, and writes them to the destination, ensuring that I/O delays never block the matching engine.

---

## 🧪 Testing

The engine includes a comprehensive test suite using **Catch2**.
 We test for:
- **ITCH Parser Correctness**: Ensuring binary messages are correctly decoded.
- **Matching Logic**: Verifying price-time priority, partial fills, and cancellations.
- **Order Book Integrity**: Ensuring the book remains balanced and consistent after complex operations.

The tests are split by implementation:
```bash
# Run tests for all implementations (using CTest)
cd build && ctest

# Or run individual test binaries
./build/test_list
./build/test_vector
./build/test_f_vector
```

---

## 🛠 Getting Started

### Prerequisites
- **Compiler**: C++20 compatible (Clang 12+ or GCC 10+).
- **Build System**: CMake 3.15+.
- **OS**: macOS (optimized for Apple Silicon) or Linux.

### Build Instructions
```bash
# Configure the project
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build
```

---

## 📁 Project Structure

```text
├── f_vector_impl/   # Optimized flat-vector + pool implementation
├── vector_impl/     # Flat-vector implementation
├── list_impl/       # Baseline map-based implementation
├── itch/            # ITCH 5.0 parser module
├── bench/           # Benchmark harness and cycle clock
├── tests/           # Unit tests (Catch2)
├── scripts/         # Benchmarking and plotting scripts
└── docs/            # Performance results and documentation
```

---

## 📖 Architectural Decisions

- **Fixed-Point Arithmetic**: Prices are handled as `int64_t` with a scale of $10^{-4}$ (matching ITCH 5.0 precision) to avoid floating-point inaccuracies and performance overhead.
- **Branch Prediction Hints**: The matching loop heavily utilizes C++20 `[[likely]]` and `[[unlikely]]` attributes to optimize the CPU instruction pipeline for the "happy path" (e.g., assuming orders will match or assuming quantities > 0).
- **Symbol Hashing**: Symbols (e.g., "AAPL    ") are parsed and hashed into `uint64_t` integers inline during ITCH ingestion, enabling O(1) dictionary lookups and avoiding `std::string` allocations entirely.
- **Memory-Mapped I/O**: The benchmark harness uses `mmap` to map binary ITCH files directly into the process's address space, enabling zero-copy parsing.
- **Callback-Based Architecture**: Trade events (fills) and order updates are dispatched via high-performance callbacks, allowing for modular and decoupled event handling without the overhead of virtual functions or message queues.
- **Asynchronous Logging**: Uses a lock-free SPSC Ring Buffer to offload logging I/O, preventing disk/console latency from affecting the matching engine's hot path.
- **Single-Threaded Core**: The matching engine is designed to run on a single pinned CPU core to avoid context switching, cache invalidation, and the overhead of multi-threaded synchronization.

---

## 📜 License
This project is open-source and available under the MIT License.
