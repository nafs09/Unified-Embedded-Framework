/// @brief CPU-only Cortex-M primitives. Clocks and timebase belong to the platform.
#include "cmsis.h"
#include <uhal/core.h>
/* This backend is single-core. A multicore platform must provide its own index,
 * shared exclusion and reset policy instead of selecting this implementation. */
uetd_u32_t uhal_core_id(void) { return 0u; }
void uhal_system_reset(void) { __DSB(); NVIC_SystemReset(); for (;;) {} }
uetd_u32_t uhal_cycle_count(void) {
#if UHAL_HAS_DWT
    /* Startup owns counter enable; reads never turn on tracing as a side effect. */
    return (DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) != 0u ? DWT->CYCCNT : 0u;
#else
    return 0u;
#endif
}
uetd_u32_t uhal_cycles_per_us(void) { return uhal_cpu_freq_hz() / 1000000u; }
void uhal_data_memory_barrier(void) { __DMB(); }
void uhal_data_sync_barrier(void) { __DSB(); }
void uhal_instruction_sync_barrier(void) { __ISB(); }
void uhal_nop(void) { __NOP(); }
