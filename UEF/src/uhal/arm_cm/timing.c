/// @file src/uhal/arm_cm/timing.c
/// @brief Cortex-M implementation boundary for the public UEF microsecond clock.
///
/// The API remains declared in UCORE, but platform time belongs to the selected
/// UHAL backend. This keeps the portable core from depending back on hardware.
#include "uef/ucore/uef_time.h"
#include "uef/uhal/uhal_core.h"

#include <stdint.h>

uef_time_us_t uef_time_now_us(void) {
    const uef_u32_t cycles_per_us = uhal_cycles_per_us();
    if (cycles_per_us == 0u) {
        /* A target must initialize its cycle source before exposing calibrated time. */
        return 0u;
    }
    return uhal_cycle_count_64() / (uef_u64_t)cycles_per_us;
}

void uef_delay_us(uef_dur_us_t duration_us) {
    const uef_u32_t cycles_per_us = uhal_cycles_per_us();
    if (duration_us == 0u || cycles_per_us == 0u) {
        return;
    }

    const uef_u64_t start = uhal_cycle_count_64();
    const uef_u64_t maximum = UINT64_MAX;
    const uef_u64_t needed = duration_us > maximum / cycles_per_us
        ? maximum
        : duration_us * cycles_per_us;
    while ((uhal_cycle_count_64() - start) < needed) {
        UHAL_NOP();
    }
}

void uef_delay_ms(uef_u32_t duration_ms) {
    uef_delay_us((uef_dur_us_t)duration_ms * 1000u);
}
