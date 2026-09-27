/// @file src/uhal/x86/core.c
/// @brief Host-only counter used by simulation and portable UEF utilities.
///
/// The counter is a simulation clock, not a measurement of CPU frequency or
/// an instruction-cycle counter. Select an embedded UHAL backend for timing
/// guarantees on real hardware.

#include "uef/uhal/uhal_core.h"

#include <time.h>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

static uef_u64_t host_counter(void) {
#if defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    if (!QueryPerformanceCounter(&counter) ||
        !QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0) {
        return 0u;
    }
    return (uef_u64_t)counter.QuadPart;
#else
    struct timespec now;
    if (timespec_get(&now, TIME_UTC) != TIME_UTC) {
        return 0u;
    }
    return (uef_u64_t)now.tv_sec * 1000000000u + (uef_u64_t)now.tv_nsec;
#endif
}

static uef_u64_t host_frequency(void) {
#if defined(_WIN32)
    LARGE_INTEGER frequency;
    return QueryPerformanceFrequency(&frequency) && frequency.QuadPart > 0
        ? (uef_u64_t)frequency.QuadPart : 0u;
#else
    return 1000000000u; /* timespec_get fallback is expressed in nanoseconds. */
#endif
}

uef_u32_t uhal_core_id(void) { return 0u; }
uef_u32_t uhal_cpu_freq_hz(void) { return uhal_cycles_per_us() * 1000000u; }
void uhal_system_reset(void) { /* A host simulation cannot reset the host OS. */ }

uef_u64_t uhal_cycle_count_64(void) { return host_counter(); }
uef_u32_t uhal_cycle_count(void) { return (uef_u32_t)uhal_cycle_count_64(); }

uef_u32_t uhal_cycles_per_us(void) {
    const uef_u64_t frequency = host_frequency();
    const uef_u64_t cycles_per_us = frequency / 1000000u;
    return cycles_per_us > 0u && cycles_per_us <= UINT32_MAX
        ? (uef_u32_t)cycles_per_us : 1u;
}
