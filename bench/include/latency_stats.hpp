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
    void add_sample(uint64_t cycles);
    void report();
    void dump_csv(const std::string& filename);

    // For testing/internal use
    size_t sample_count() const { return samples.size(); }

private:
    std::vector<uint64_t> samples;
};

} // namespace bench
} // namespace engine
