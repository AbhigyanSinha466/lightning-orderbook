#include "cycle_clock.hpp"
#include <dlfcn.h>
#include <iostream>
#include <vector>

namespace engine {
namespace bench {

#define KPC_CLASS_FIXED_MASK (1 << 0)

// kpc_get_thread_counters(int tid, uint32_t *inoutcount, uint64_t *buf)
// On some versions tid 0 means current thread.
typedef int (*kpc_get_thread_counters_t)(int, uint32_t *, uint64_t *);
typedef int (*kpc_set_thread_counting_t)(uint32_t classes);
typedef uint32_t (*kpc_get_config_count_t)(uint32_t classes);

static kpc_get_thread_counters_t kpc_get_thread_counters = nullptr;
static kpc_set_thread_counting_t kpc_set_thread_counting = nullptr;
static kpc_get_config_count_t kpc_get_config_count = nullptr;

bool CycleClock::initialized = false;

bool CycleClock::init() {
    if (initialized) return true;

    void* kperf = dlopen("/System/Library/PrivateFrameworks/kperf.framework/kperf", RTLD_LAZY);
    if (!kperf) {
        std::cerr << "Failed to load kperf: " << dlerror() << std::endl;
        return false;
    }

    // Try multiple possible symbol names for current thread counters
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

uint64_t CycleClock::get_cycles() {
    if (!initialized) return 0;

    uint32_t count = 2; // Fixed counters: usually 0 is cycles, 1 is instructions
    uint64_t counters[2];
    // tid = 0 usually refers to the current thread in kpc_get_thread_counters
    if (kpc_get_thread_counters(0, &count, counters) == 0) {
        return counters[0]; // CPU Cycles
    }
    return 0;
}

} // namespace bench
} // namespace engine
