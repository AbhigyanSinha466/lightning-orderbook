#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace engine {
namespace bench {

/**
 * @brief LatencyStats collects latency samples and computes statistics.
 */
class LatencyStats {
public:
    void add_sample(uint64_t nanoseconds);
    void report();

    // For testing/internal use
    size_t sample_count() const { return latencies.size(); }

private:
    std::vector<uint64_t> latencies;
};

} // namespace bench
} // namespace engine
