/// @file src/uhal/arm_cm/core.c
/// @brief Source scaffold for the V1.1 public contract in uef/uhal/uhal_core.h.
///
/// Implementation intent: Bind core identity, reset, cycle-counter, and barriers to the
///   selected CMSIS device without leaking device names into portable modules.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uhal/uhal_core.h>

uef_u32_t uhal_core_id(void) {
    /* TODO(UEF Cortex-M):
     * Read the target core identity and cycle counter using supported CMSIS facilities;
     * document enablement, wraparound, and conversion to microseconds. Implement this
     * contract for the selected Cortex-M CMSIS device without assuming a particular vendor
     * register map. Keep interrupt and register side effects documented, bounded, and safe
     * for the active target.
     */
    return 0;
}

uef_u32_t uhal_cpu_freq_hz(void) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    return 0;
}

void uhal_system_reset(void) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
}

uef_u32_t uhal_cycle_count(void) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    return 0;
}

uef_u64_t uhal_cycle_count_64(void) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    return 0;
}

uef_u32_t uhal_cycles_per_us(void) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    return 0;
}
