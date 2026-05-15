#include "cycle_clock.hpp"
#include <dlfcn.h>
#include <iostream>
#include <chrono>
#include <thread>

namespace engine {
namespace bench {

#define KPC_CLASS_FIXED_MASK (1 << 0)

// kpc_get_thread_counters(int tid, uint32_t *inoutcount, uint64_t *buf)
typedef int (*kpc_get_thread_counters_t)(int, uint32_t *, uint64_t *);
typedef int (*kpc_set_thread_counting_t)(uint32_t classes);
typedef uint32_t (*kpc_get_config_count_t)(uint32_t classes);

static kpc_get_thread_counters_t kpc_get_thread_counters = nullptr;
static kpc_set_thread_counting_t kpc_set_thread_counting = nullptr;
static kpc_get_config_count_t kpc_get_config_count = nullptr;

bool CycleClock::initialized = false;
double CycleClock::cached_ns_per_cycle = 1.0;

bool CycleClock::init() {
    if (initialized) return true;

    void* kperf = dlopen("/System/Library/PrivateFrameworks/kperf.framework/kperf", RTLD_LAZY);
    if (!kperf) {
        std::cerr << "Failed to load kperf: " << dlerror() << std::endl;
        return false;
    }

    kpc_get_thread_counters = (kpc_get_thread_counters_t)dlsym(kperf, "kpc_get_thread_counters");
    if (!kpc_get_thread_counters) {
        kpc_get_thread_counters = (kpc_get_thread_counters_t)dlsym(kperf, "kpc_get_curthread_counters");
    }

    kpc_set_thread_counting = (kpc_set_thread_counting_t)dlsym(kperf, "kpc_set_thread_counting");
    kpc_get_config_count = (kpc_get_config_count_t)dlsym(kperf, "kpc_get_config_count");

    if (!kpc_get_thread_counters || !kpc_set_thread_counting || !kpc_get_config_count) {
        std::cerr << "Failed to resolve symbols in kperf." << std::endl;
        return false;
    }

    if (kpc_set_thread_counting(KPC_CLASS_FIXED_MASK) != 0) {
        std::cerr << "Failed to enable thread counting. Ensure you are running with root privileges (sudo)." << std::endl;
        return false;
    }

    initialized = true;
    return true;
}

void CycleClock::calibrate() {
    auto start_ns = std::chrono::high_resolution_clock::now();
    uint64_t start_cycles = get_cycles();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto end_ns = std::chrono::high_resolution_clock::now();
    uint64_t end_cycles = get_cycles();
    
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_ns - start_ns).count();
    uint64_t delta_cycles = end_cycles - start_cycles;
    
    if (delta_cycles > 0) {
        cached_ns_per_cycle = static_cast<double>(duration_ns) / delta_cycles;
    }
}

double CycleClock::ns_per_cycle() {
    return cached_ns_per_cycle;
}

uint64_t CycleClock::get_cycles() {
    if (!initialized) return 0;

    uint32_t count = 2;
    uint64_t counters[2];
    if (kpc_get_thread_counters(0, &count, counters) == 0) {
        return counters[0];
    }
    return 0;
}

} // namespace bench
} // namespace engine
