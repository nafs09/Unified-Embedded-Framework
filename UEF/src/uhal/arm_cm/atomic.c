/// @file src/uhal/arm_cm/atomic.c
/// @brief Source scaffold for the V1.1 public contract in uef/uhal/uhal_atomic.h.
///
/// Implementation intent: Use architecture-safe exclusive instructions or C11 atomics with
///   documented ISR and memory-order guarantees.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uhal/uhal_atomic.h>

bool uhal_atomic_cas_u32(
    volatile uef_u32_t* target,
    uef_u32_t expected,
    uef_u32_t desired
) {
    /* TODO(UEF Cortex-M):
     * Use the Cortex-M atomic/exclusive instructions or a correctly scoped critical
     * section, preserve the documented memory ordering, and handle alignment and interrupt
     * nesting. Implement this contract for the selected Cortex-M CMSIS device without
     * assuming a particular vendor register map. Keep interrupt and register side effects
     * documented, bounded, and safe for the active target.
     */
    (void)target;
    (void)expected;
    (void)desired;
    return false;
}

uef_u32_t uhal_atomic_fetch_add_u32(
    volatile uef_u32_t* target,
    uef_u32_t val
) {
    /* TODO(UEF Cortex-M):
     * Use the Cortex-M atomic/exclusive instructions or a correctly scoped critical
     * section, preserve the documented memory ordering, and handle alignment and interrupt
     * nesting. Implement this contract for the selected Cortex-M CMSIS device without
     * assuming a particular vendor register map. Keep interrupt and register side effects
     * documented, bounded, and safe for the active target.
     */
    (void)target;
    (void)val;
    return 0;
}

uef_u32_t uhal_atomic_fetch_and_u32(
    volatile uef_u32_t* target,
    uef_u32_t val
) {
    /* TODO(UEF Cortex-M):
     * Use the Cortex-M atomic/exclusive instructions or a correctly scoped critical
     * section, preserve the documented memory ordering, and handle alignment and interrupt
     * nesting. Implement this contract for the selected Cortex-M CMSIS device without
     * assuming a particular vendor register map. Keep interrupt and register side effects
     * documented, bounded, and safe for the active target.
     */
    (void)target;
    (void)val;
    return 0;
}

uef_u32_t uhal_atomic_fetch_or_u32(
    volatile uef_u32_t* target,
    uef_u32_t val
) {
    /* TODO(UEF Cortex-M):
     * Use the Cortex-M atomic/exclusive instructions or a correctly scoped critical
     * section, preserve the documented memory ordering, and handle alignment and interrupt
     * nesting. Implement this contract for the selected Cortex-M CMSIS device without
     * assuming a particular vendor register map. Keep interrupt and register side effects
     * documented, bounded, and safe for the active target.
     */
    (void)target;
    (void)val;
    return 0;
}

uef_u32_t uhal_atomic_load_u32(
    const volatile uef_u32_t* src
) {
    /* TODO(UEF Cortex-M):
     * Use the Cortex-M atomic/exclusive instructions or a correctly scoped critical
     * section, preserve the documented memory ordering, and handle alignment and interrupt
     * nesting. Implement this contract for the selected Cortex-M CMSIS device without
     * assuming a particular vendor register map. Keep interrupt and register side effects
     * documented, bounded, and safe for the active target.
     */
    (void)src;
    return 0;
}

void uhal_atomic_store_u32(
    volatile uef_u32_t* dst,
    uef_u32_t val
) {
    /* TODO(UEF Cortex-M):
     * Use the Cortex-M atomic/exclusive instructions or a correctly scoped critical
     * section, preserve the documented memory ordering, and handle alignment and interrupt
     * nesting. Implement this contract for the selected Cortex-M CMSIS device without
     * assuming a particular vendor register map. Keep interrupt and register side effects
     * documented, bounded, and safe for the active target.
     */
    (void)dst;
    (void)val;
}
