/// @file src/uhal/arm_cm/cache.c
/// @brief Source scaffold for the V1.2 public contract in uef/uhal/uhal_cache.h.
///
/// Implementation intent: Implement cache-line-aligned maintenance only on cache-equipped
///   targets; keep no-cache targets as compile-time no-ops.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uhal/uhal_cache.h>

/* These functions are declared only when the selected device has D-cache. */
#if UHAL_HAS_DCACHE

void uhal_cache_clean(
    const void* addr,
    uef_u32_t size
) {
    /* TODO(UEF Cortex-M):
     * Implement cache-line aligned maintenance and enable sequencing for cache-capable
     * parts; compile the declarations only when the target exposes data/instruction cache.
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    (void)addr;
    (void)size;
}

void uhal_cache_invalidate(
    void* addr,
    uef_u32_t size
) {
    /* TODO(UEF Cortex-M):
     * Implement cache-line aligned maintenance and enable sequencing for cache-capable
     * parts; compile the declarations only when the target exposes data/instruction cache.
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    (void)addr;
    (void)size;
}

void uhal_cache_clean_invalidate(
    void* addr,
    uef_u32_t size
) {
    /* TODO(UEF Cortex-M):
     * Implement cache-line aligned maintenance and enable sequencing for cache-capable
     * parts; compile the declarations only when the target exposes data/instruction cache.
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    (void)addr;
    (void)size;
}

void uhal_dcache_enable(void) {
    /* TODO(UEF Cortex-M):
     * Implement cache-line aligned maintenance and enable sequencing for cache-capable
     * parts; compile the declarations only when the target exposes data/instruction cache.
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
}

void uhal_icache_enable(void) {
    /* TODO(UEF Cortex-M):
     * Implement cache-line aligned maintenance and enable sequencing for cache-capable
     * parts; compile the declarations only when the target exposes data/instruction cache.
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
}

#endif /* UHAL_HAS_DCACHE */
