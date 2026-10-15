/// @file src/uhal/x86/timing.c
/// @brief Host-simulation implementation of the public UETD microsecond clock.
///
/// Host time is useful for portable simulation and diagnostics. It is not a
/// measurement of firmware execution cycles or a real-time scheduling guarantee.
#include "ucore/time.h"
#include "uhal/core.h"
#include <stdint.h>

uetd_time_us_t uetd_time_now_us(void) {
    const uetd_u32_t cycles_per_us = uhal_cycles_per_us();
    if (cycles_per_us == 0u) { return 0u; }
    return uhal_cycle_count_64() / (uetd_u64_t)cycles_per_us;
}

void uetd_delay_us(uetd_dur_us_t duration_us) {
    const uetd_u32_t cycles_per_us = uhal_cycles_per_us();
    if (duration_us == 0u || cycles_per_us == 0u) { return; }
    const uetd_u64_t start = uhal_cycle_count_64();
    const uetd_u64_t maximum = UINT64_MAX;
    const uetd_u64_t needed = duration_us > maximum / cycles_per_us ? maximum : duration_us * cycles_per_us;
    while ((uhal_cycle_count_64() - start) < needed) { UHAL_NOP(); }
}

void uetd_delay_ms(uetd_u32_t duration_ms) { uetd_delay_us((uetd_dur_us_t)duration_ms * 1000u); }
