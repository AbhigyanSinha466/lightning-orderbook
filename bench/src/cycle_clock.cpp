#include "cycle_clock.hpp"
#include <dlfcn.h>
#include <iostream>
#include <vector>
#include <mach/mach_time.h>
#include <unistd.h>
#include <pthread/qos.h>

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
double CycleClock::ns_per_cycle = 0.227272727; // Default 4.4 GHz

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
    // 1. Set QoS to force P-cores and maximum frequency
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    // 2. Warm up the CPU more aggressively
    for (volatile int i = 0; i < 50000000; ++i) {}

    // 3. Initial readings
    uint64_t start_time = mach_absolute_time();
    uint64_t start_cycles = get_cycles();

    // 4. Busy wait for ~100ms to allow measurement window
    // usleep() would deschedule the thread, making thread-local counters stop.
    uint64_t elapsed_ns = 0;
    uint64_t target_ns = 100000000; // 100ms
    while (elapsed_ns < target_ns) {
        uint64_t now = mach_absolute_time();
        elapsed_ns = ((now - start_time) * timebase.numer) / timebase.denom;
    }

    // 5. Final readings
    uint64_t end_cycles = get_cycles();
    uint64_t end_time = mach_absolute_time();

    uint64_t final_elapsed_mach = end_time - start_time;
    uint64_t final_elapsed_ns = (final_elapsed_mach * timebase.numer) / timebase.denom;
    uint64_t elapsed_cycles = end_cycles - start_cycles;

    if (final_elapsed_ns > 0 && elapsed_cycles > 0) {
        ns_per_cycle = (double)final_elapsed_ns / (double)elapsed_cycles;
        double ghz = 1.0 / ns_per_cycle;
        std::cerr << "CycleClock Calibrated: " << ghz << " GHz (" << ns_per_cycle << " ns/cycle)" << std::endl;
    }
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
