/// @file src/uhal/x86/clock.c
/// @brief Simulation clock and reset stubs for host builds.

#include "uef/uhal/uhal_clock.h"

uef_u32_t uhal_clk_freq_hz(uhal_clk_t clock) {
    (void)clock;
    return 1000000u; /* Simulation tick domain; not a hardware peripheral clock. */
}

/* Clock gating and reset have no hardware side effect in the host simulation backend. */
/* TODO(host-clock-model): model per-peripheral enabled/reset state if simulator tests need to
 * detect invalid use-before-enable sequences; keep this state separate from target drivers. */
void uhal_clk_periph_enable(uhal_periph_clk_t peripheral) { (void)peripheral; }
void uhal_clk_periph_disable(uhal_periph_clk_t peripheral) { (void)peripheral; }
void uhal_reset_periph_assert(uhal_periph_clk_t peripheral) { (void)peripheral; }
void uhal_reset_periph_release(uhal_periph_clk_t peripheral) { (void)peripheral; }
uef_u32_t uhal_reset_cause(void) { return UHAL_RESET_POWER_ON; }
