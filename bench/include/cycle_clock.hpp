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
     * @brief Returns the current CPU cycle count.
     */
    static uint64_t get_cycles();

private:
    static bool initialized;
};

} // namespace bench
} // namespace engine
