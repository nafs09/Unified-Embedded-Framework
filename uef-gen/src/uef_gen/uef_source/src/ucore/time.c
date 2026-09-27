/// @file src/ucore/time.c
/// @brief Common microsecond time and startup-delay helpers.

#include "uef/ucore/uef_time.h"
#include "uef/uhal/uhal_core.h"

#include <stdint.h>

uef_time_us_t uef_time_now_us(void) {
    const uef_u32_t cycles_per_us = uhal_cycles_per_us();
    if (cycles_per_us == 0u) {
        /* A target must initialize its cycle source before exposing calibrated microseconds. */
        return 0u;
    }
    /* Use the 64-bit counter to extend the useful time range beyond one hardware wrap. */
    return uhal_cycle_count_64() / (uef_u64_t)cycles_per_us;
}

void uef_delay_us(uef_dur_us_t duration_us) {
    const uef_u32_t cycles_per_us = uhal_cycles_per_us();
    if (duration_us == 0u || cycles_per_us == 0u) {
        return;
    }

    const uef_u64_t start = uhal_cycle_count_64();
    const uef_u64_t maximum = UINT64_MAX;
    /* Saturate the cycle conversion instead of wrapping a very long requested delay shorter. */
    const uef_u64_t needed = duration_us > maximum / cycles_per_us
        ? maximum
        : duration_us * cycles_per_us;

    /* Unsigned subtraction remains correct when the counter wraps once. */
    while ((uhal_cycle_count_64() - start) < needed) {
        UHAL_NOP();
    }
}

void uef_delay_ms(uef_u32_t duration_ms) {
    /* Delegate to the single microsecond implementation so wrap/clock handling stays consistent. */
    uef_delay_us((uef_dur_us_t)duration_ms * 1000u);
}
