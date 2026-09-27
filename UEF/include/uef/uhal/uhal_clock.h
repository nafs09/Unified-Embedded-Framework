/// @file include/uef/uhal/uhal_clock.h
/// @brief Clock and reset management.
/// Device startup configures the clock tree before `main`; application code queries clocks and
/// uses explicit peripheral enable/reset hooks rather than reconfiguring the tree implicitly.

#ifndef UHAL_CLOCK_H
#define UHAL_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

typedef enum {
    UHAL_CLK_SYSCLK,
    UHAL_CLK_HCLK,
    UHAL_CLK_APB1,
    UHAL_CLK_APB2,
    UHAL_CLK_APB3,
    UHAL_CLK_AHB,
    UHAL_CLK_ADC,
    UHAL_CLK_USB,
    UHAL_CLK_PLL_Q,
} uhal_clk_t;

/* Query — called by UPAL to compute prescalers and baud rates */
uef_u32_t uhal_clk_freq_hz(uhal_clk_t clk);

/* Peripheral clock gate control */
typedef uef_u32_t uhal_periph_clk_t;   /* device-specific opaque value */
void uhal_clk_periph_enable(uhal_periph_clk_t periph);
void uhal_clk_periph_disable(uhal_periph_clk_t periph);
void uhal_reset_periph_assert(uhal_periph_clk_t periph);
void uhal_reset_periph_release(uhal_periph_clk_t periph);

/* Reset cause — read once at startup before any peripheral init */
typedef enum {
    UHAL_RESET_POWER_ON   = (1u << 0),
    UHAL_RESET_PIN        = (1u << 1),
    UHAL_RESET_SOFTWARE   = (1u << 2),
    UHAL_RESET_IWDG       = (1u << 3),
    UHAL_RESET_WWDG       = (1u << 4),
    UHAL_RESET_LOW_POWER  = (1u << 5),
    UHAL_RESET_FAULT      = (1u << 6),  /* set by fault handler via RTC backup */
} uhal_reset_cause_t;

uef_u32_t uhal_reset_cause(void);  /* reads and clears RCC reset flags */

#ifdef __cplusplus
}
#endif

#endif /* UHAL_CLOCK_H */
