#include "matching_engine.hpp"
#include "itch_parser.hpp"
#include "replay_harness.hpp"
#include "latency_stats.hpp"
#include "cycle_clock.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <iomanip>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <itch_binary_file>" << std::endl;
        return 1;
    }

    if (!engine::bench::CycleClock::init()) {
        std::cerr << "ERROR: Failed to initialize CycleClock. Did you run with sudo?" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = st.st_size;
    void* addr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    const uint8_t* data = static_cast<const uint8_t*>(addr);

    engine::MatchingEngine engine;
    engine::itch::ItchParser parser;
    engine::bench::LatencyStats stats;
    engine::bench::ReplayHarness harness(engine, parser, stats);

    // Track total fills for the report
    size_t fill_count = 0;
    engine.set_on_fill([&](const engine::FillEvent&) {
        fill_count++;
    });

    std::cerr << "Starting replay of " << file_size << " bytes..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    size_t consumed = parser.parse(data, file_size);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "\n================================================\n";
    std::cout << " BENCHMARK REPORT\n";
    std::cout << "================================================\n";
    std::cout << std::left << std::setw(25) << "Total Bytes:" << file_size << "\n";
    std::cout << std::left << std::setw(25) << "Bytes Consumed:" << consumed << "\n";
    std::cout << std::left << std::setw(25) << "Total Fills:" << fill_count << "\n";
    std::cout << std::left << std::setw(25) << "Wall-clock Time:" << elapsed << " ms\n";
    if (elapsed > 0) {
        double mps = static_cast<double>(stats.sample_count()) / (elapsed / 1000.0);
        std::cout << std::left << std::setw(25) << "Throughput:" << mps << " msgs/sec\n";
    }

    stats.report();
    stats.dump_csv("latency_results.csv");

    munmap(addr, file_size);
    close(fd);

    return 0;
}
