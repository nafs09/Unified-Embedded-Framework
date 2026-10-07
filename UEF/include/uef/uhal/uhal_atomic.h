/// @file include/uef/uhal/uhal_atomic.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UHAL_ATOMIC_H
#define UHAL_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

/* Full-barrier 32-bit operations on naturally aligned uef_u32_t objects.
 * The Cortex-M backend currently serializes each operation with a brief PRIMASK
 * critical section, so it is safe across tasks and maskable ISRs on one core.
 * Calls from NMI/fault handlers and multicore sharing are outside this contract.
 */
bool     uhal_atomic_cas_u32(volatile uef_u32_t* target,
                               uef_u32_t expected, uef_u32_t desired);
uef_u32_t uhal_atomic_fetch_add_u32(volatile uef_u32_t* target, uef_u32_t val);
uef_u32_t uhal_atomic_fetch_and_u32(volatile uef_u32_t* target, uef_u32_t val);
uef_u32_t uhal_atomic_fetch_or_u32(volatile uef_u32_t* target,  uef_u32_t val);
uef_u32_t uhal_atomic_load_u32(const volatile uef_u32_t* src);
void      uhal_atomic_store_u32(volatile uef_u32_t* dst, uef_u32_t val);

#ifdef __cplusplus
}
#endif

#endif /* UHAL_ATOMIC_H */
