/// @file include/uef/uhal/uhal_core.h
/// @brief CPU identification, reset, and core utilities.

#ifndef UHAL_CORE_H
#define UHAL_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/uhal/target.h"
#include "uef/ucore/uef_status.h"

/* Core identification */
uef_u32_t uhal_core_id(void);           /* unique core ID where available */
uef_u32_t uhal_cpu_freq_hz(void);       /* current SYSCLK in Hz */
void      uhal_system_reset(void);      /* software reset */

/* CPU cycle counter — uses DWT on Cortex-M; RDTSC on x86 */
uef_u32_t uhal_cycle_count(void);       /* wraps at 2^32 cycles */
uef_u64_t uhal_cycle_count_64(void);    /* monotonic, never wraps */
uef_u32_t uhal_cycles_per_us(void);     /* cpu_freq_hz / 1e6 */

/* Memory barriers */
#define UHAL_DMB()   __DMB()
#define UHAL_DSB()   __DSB()
#define UHAL_ISB()   __ISB()
/* x86 fallback: _mm_mfence() / _mm_sfence() / _mm_lfence() */

/* NOP — for timing alignment */
#define UHAL_NOP()   __NOP()

#ifdef __cplusplus
}
#endif

#endif /* UHAL_CORE_H */
