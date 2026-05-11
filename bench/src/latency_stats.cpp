#include "latency_stats.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <fstream>

namespace engine {
namespace bench {

void LatencyStats::add_sample(uint64_t nanoseconds) {
    latencies.push_back(nanoseconds);
}

void LatencyStats::dump_csv(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to open " << filename << " for writing latency data." << std::endl;
        return;
    }
    out << "latency_ns\n";
    for (auto lat : latencies) {
        out << lat << "\n";
    }
}

void LatencyStats::report() {
    if (latencies.empty()) {
        std::cout << "No samples collected." << std::endl;
        return;
    }

    std::sort(latencies.begin(), latencies.end());

    auto sum = std::accumulate(latencies.begin(), latencies.end(), 0ULL);
    double mean = static_cast<double>(sum) / latencies.size();

    auto percentile = [&](double p) {
        if (p <= 0.0) return latencies.front();
        size_t idx = static_cast<size_t>(std::ceil(p / 100.0 * latencies.size())) - 1;
        return latencies[std::min(idx, latencies.size() - 1)];
    };

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n------------------------------------------------\n";
    std::cout << " LATENCY STATISTICS (nanoseconds)\n";
    std::cout << "------------------------------------------------\n";
    std::cout << std::left << std::setw(20) << "Count:" << latencies.size() << "\n";
    std::cout << std::left << std::setw(20) << "Min:" << latencies.front() << "\n";
    std::cout << std::left << std::setw(20) << "Max:" << latencies.back() << "\n";
    std::cout << std::left << std::setw(20) << "Mean:" << mean << "\n";
    std::cout << std::left << std::setw(20) << "50th (Median):" << percentile(50) << "\n";
    std::cout << std::left << std::setw(20) << "90th:" << percentile(90) << "\n";
    std::cout << std::left << std::setw(20) << "95th:" << percentile(95) << "\n";
    std::cout << std::left << std::setw(20) << "99th:" << percentile(99) << "\n";
    std::cout << std::left << std::setw(20) << "99.9th:" << percentile(99.9) << "\n";
    std::cout << "------------------------------------------------\n" << std::endl;
}

} // namespace bench
} // namespace engine
