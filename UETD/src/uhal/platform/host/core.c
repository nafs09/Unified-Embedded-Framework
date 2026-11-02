/// @file src/uhal/platform/host/core.c
/// @brief Host-only counter used by simulation and portable UETD utilities.
///
/// The counter is a simulation clock, not a measurement of CPU frequency or
/// an instruction-cycle counter. Select an embedded UHAL backend for timing
/// guarantees on real hardware.

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "uhal/core.h"
#include <time.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

static uetd_u64_t host_counter(void) {
#if defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    if (!QueryPerformanceCounter(&counter) || !QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0) { return 0u; }
    const uetd_u64_t ticks = (uetd_u64_t)counter.QuadPart;
    const uetd_u64_t hz = (uetd_u64_t)frequency.QuadPart;
    /* Divide before multiplying the uptime: no truncated ticks/us scale. */
    return (ticks / hz) * 1000000u + ((ticks % hz) * 1000000u) / hz;
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) { return 0u; }
    return (uetd_u64_t)now.tv_sec * 1000000u + (uetd_u64_t)now.tv_nsec / 1000u;
#endif
}

uetd_u32_t uhal_core_id(void) { return 0u; }

uetd_u32_t uhal_cpu_freq_hz(void) { return 0u; } /* Host CPU frequency is not simulated. */

void uhal_system_reset(void) { /* A host simulation cannot reset the host OS. */ }

uetd_u64_t uhal_cycle_count_64(void) { return host_counter(); }

uetd_u32_t uhal_cycle_count(void) { return (uetd_u32_t)uhal_cycle_count_64(); }

uetd_u32_t uhal_cycles_per_us(void) { return 1u; } /* Simulated counter ticks, not CPU cycles. */

/* Host barriers provide real inter-thread ordering; they do not emulate device
 * cache maintenance or an embedded instruction pipeline. */
void uhal_data_memory_barrier(void) {
#if defined(_WIN32)
    MemoryBarrier();
#else
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
#endif
}
void uhal_data_sync_barrier(void) { uhal_data_memory_barrier(); }
void uhal_instruction_sync_barrier(void) { uhal_data_memory_barrier(); }
void uhal_nop(void) {
#if defined(_WIN32)
    YieldProcessor();
#else
    __asm__ volatile("" ::: "memory");
#endif
}
