#pragma once
#include <cstdint>

namespace engine {
namespace bench {

/**
 * @brief CycleClock provides access to high-precision CPU cycle counters on Apple Silicon.
 * Requires running with root privileges (sudo).
 */
class CycleClock {
public:
    /**
     * @brief Initializes the cycle counter. Must be called once before get_cycles().
     * @return true if initialization was successful.
     */
    static bool init();

    /**
     * @brief Calibrates the cycle counter to determine the nanoseconds per cycle.
     */
    static void calibrate();

    /**
     * @brief Returns the calibrated nanoseconds per cycle.
     */
    static double ns_per_cycle();

    /**
     * @brief Returns the current CPU cycle count.
     */
    static uint64_t get_cycles();

private:
    static bool initialized;
    static double cached_ns_per_cycle;
};

} // namespace bench
} // namespace engine
