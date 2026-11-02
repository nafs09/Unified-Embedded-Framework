/// @brief Cache maintenance for Cortex-M devices that actually expose caches.
#include "cmsis.h"
#include <uhal/cache.h>
#include <limits.h>

#if UHAL_HAS_DCACHE
/* Partial-line invalidation can discard unrelated dirty data. DMA buffers and
 * their lengths must be whole cache lines; ownership transfers at these calls. */
static bool valid_range(const void* address, uetd_u32_t size) {
    const uintptr_t start = (uintptr_t)address;
    return address != NULL && UHAL_CACHE_LINE_SIZE != 0u &&
        start % UHAL_CACHE_LINE_SIZE == 0u && size % UHAL_CACHE_LINE_SIZE == 0u &&
        size <= INT32_MAX && start <= UINTPTR_MAX - size;
}
uetd_status_t uhal_cache_clean(const void* address, uetd_u32_t size) {
    if (size == 0u) { return UETD_OK; }
    if (!valid_range(address, size)) { return UETD_CACHE_ERROR; }
    SCB_CleanDCache_by_Addr((void*)address, (int32_t)size); __DSB();
    return UETD_OK;
}
uetd_status_t uhal_cache_invalidate(void* address, uetd_u32_t size) {
    if (size == 0u) { return UETD_OK; }
    if (!valid_range(address, size)) { return UETD_CACHE_ERROR; }
    SCB_InvalidateDCache_by_Addr(address, (int32_t)size); __DSB(); __ISB();
    return UETD_OK;
}
uetd_status_t uhal_cache_clean_invalidate(void* address, uetd_u32_t size) {
    if (size == 0u) { return UETD_OK; }
    if (!valid_range(address, size)) { return UETD_CACHE_ERROR; }
    SCB_CleanInvalidateDCache_by_Addr(address, (int32_t)size); __DSB(); __ISB();
    return UETD_OK;
}
void uhal_dcache_enable(void) { SCB_EnableDCache(); __DSB(); __ISB(); }
#endif
#if UHAL_HAS_ICACHE
void uhal_icache_enable(void) { SCB_EnableICache(); __DSB(); __ISB(); }
#endif
