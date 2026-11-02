/// @file src/uhal/x86/core.c
/// @brief Host-only counter used by simulation and portable UETD utilities.
///
/// The counter is a simulation clock, not a measurement of CPU frequency or
/// an instruction-cycle counter. Select an embedded UHAL backend for timing
/// guarantees on real hardware.

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
    return (uetd_u64_t)counter.QuadPart;
#else
    struct timespec now;
    if (timespec_get(&now, TIME_UTC) != TIME_UTC) { return 0u; }
    return (uetd_u64_t)now.tv_sec * 1000000000u + (uetd_u64_t)now.tv_nsec;
#endif
}

static uetd_u64_t host_frequency(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    return QueryPerformanceFrequency(&frequency) && frequency.QuadPart > 0 ? (uetd_u64_t)frequency.QuadPart : 0u;
#else
    return 1000000000u; /* timespec_get fallback is expressed in nanoseconds. */
#endif
}

uetd_u32_t uhal_core_id(void) { return 0u; }

uetd_u32_t uhal_cpu_freq_hz(void) { return uhal_cycles_per_us() * 1000000u; }

void uhal_system_reset(void) { /* A host simulation cannot reset the host OS. */ }

uetd_u64_t uhal_cycle_count_64(void) { return host_counter(); }

uetd_u32_t uhal_cycle_count(void) { return (uetd_u32_t)uhal_cycle_count_64(); }

uetd_u32_t uhal_cycles_per_us(void) {
    const uetd_u64_t frequency = host_frequency();
    const uetd_u64_t cycles_per_us = frequency / 1000000u;
    return cycles_per_us > 0u && cycles_per_us <= UINT32_MAX ? (uetd_u32_t)cycles_per_us : 1u;
}
