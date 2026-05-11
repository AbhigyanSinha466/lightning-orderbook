#include "latency_stats.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iomanip>
#include <cmath>
#include <fstream>

namespace engine {
namespace bench {

// Assuming M4 P-core boost frequency of ~4.4 GHz
// 1 / 4.4 GHz = 0.227272727 ns/cycle
constexpr double NS_PER_CYCLE = 0.227272727;

void LatencyStats::add_sample(uint64_t cycles) {
    samples.push_back(cycles);
}

void LatencyStats::dump_csv(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to open " << filename << " for writing latency data." << std::endl;
        return;
    }
    out << "latency_cycles,latency_ns\n";
    for (auto s : samples) {
        out << s << "," << (static_cast<double>(s) * NS_PER_CYCLE) << "\n";
    }
}

void LatencyStats::report() {
    if (samples.empty()) {
        std::cout << "No samples collected." << std::endl;
        return;
    }

    std::sort(samples.begin(), samples.end());

    auto sum = std::accumulate(samples.begin(), samples.end(), 0ULL);
    double mean = static_cast<double>(sum) / samples.size();

    auto percentile = [&](double p) {
        if (p <= 0.0) return samples.front();
        size_t idx = static_cast<size_t>(std::ceil(p / 100.0 * samples.size())) - 1;
        return samples[std::min(idx, samples.size() - 1)];
    };

    auto to_ns = [](uint64_t c) { return static_cast<double>(c) * NS_PER_CYCLE; };

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n------------------------------------------------\n";
    std::cout << " LATENCY STATISTICS (CPU Cycles & Nanoseconds)\n";
    std::cout << "------------------------------------------------\n";
    std::cout << std::left << std::setw(20) << "Count:" << samples.size() << "\n";
    std::cout << std::left << std::setw(20) << "Min:" << samples.front() << " cycles (" << to_ns(samples.front()) << " ns)\n";
    std::cout << std::left << std::setw(20) << "Max:" << samples.back() << " cycles (" << to_ns(samples.back()) << " ns)\n";
    std::cout << std::left << std::setw(20) << "Mean:" << mean << " cycles (" << (mean * NS_PER_CYCLE) << " ns)\n";
    std::cout << std::left << std::setw(20) << "50th (Median):" << percentile(50) << " cycles (" << to_ns(percentile(50)) << " ns)\n";
    std::cout << std::left << std::setw(20) << "90th:" << percentile(90) << " cycles (" << to_ns(percentile(90)) << " ns)\n";
    std::cout << std::left << std::setw(20) << "95th:" << percentile(95) << " cycles (" << to_ns(percentile(95)) << " ns)\n";
    std::cout << std::left << std::setw(20) << "99th:" << percentile(99) << " cycles (" << to_ns(percentile(99)) << " ns)\n";
    std::cout << std::left << std::setw(20) << "99.9th:" << percentile(99.9) << " cycles (" << to_ns(percentile(99.9)) << " ns)\n";
    std::cout << "------------------------------------------------\n";
    std::cout << " * Note: ns conversion assumes 4.4 GHz CPU frequency.\n";
    std::cout << "------------------------------------------------\n" << std::endl;
}

} // namespace bench
} // namespace engine
