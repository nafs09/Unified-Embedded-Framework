/// @file include/uhal/clock.h
/// @brief Clock and reset management.
/// Device startup configures the clock tree before `main`; application code queries clocks and
/// uses explicit peripheral enable/reset hooks rather than reconfiguring the tree implicitly.

#ifndef UHAL_CLOCK_H
#define UHAL_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"

/* Platform-generated IDs describe real clock domains. CPU architecture does
 * not prescribe APB buses, PLL outputs or peripheral kernel clocks. */
typedef uetd_u32_t uhal_clk_t;
#define UHAL_CLK_CPU ((uhal_clk_t)0u)
#define UHAL_CLK_SYSTEM ((uhal_clk_t)1u)
#define UHAL_CLK_MONOTONIC ((uhal_clk_t)2u)
#define UHAL_CLK_PLATFORM_BASE ((uhal_clk_t)0x100u)

/* Query — called by UPAL to compute prescalers and baud rates */
/// Return the current frequency of a peripheral clock in hertz.
uetd_u32_t uhal_clk_freq_hz(uhal_clk_t clk);

/* Peripheral clock gate control */
typedef uetd_u32_t uhal_periph_clk_t; /* device-specific opaque value */
/// Enable the selected peripheral clock.
void uhal_clk_periph_enable(uhal_periph_clk_t periph);
/// Disable the selected peripheral clock.
void uhal_clk_periph_disable(uhal_periph_clk_t periph);
/// Hold the selected peripheral in reset.
void uhal_reset_periph_assert(uhal_periph_clk_t periph);
/// Release the selected peripheral from reset.
void uhal_reset_periph_release(uhal_periph_clk_t periph);

/* Reset cause — read once at startup before any peripheral init */
typedef enum {
    UHAL_RESET_POWER_ON = (1u << 0),
    UHAL_RESET_PIN = (1u << 1),
    UHAL_RESET_SOFTWARE = (1u << 2),
    UHAL_RESET_IWDG = (1u << 3),
    UHAL_RESET_WWDG = (1u << 4),
    UHAL_RESET_LOW_POWER = (1u << 5),
    UHAL_RESET_FAULT = (1u << 6), /* set by fault handler via RTC backup */
} uhal_reset_cause_t;

/// Read and clear the reset-cause flags recorded by the platform.
uetd_u32_t uhal_reset_cause(void); /* platform maps its reset flags */

#ifdef __cplusplus
}
#endif

#endif /* UHAL_CLOCK_H */
