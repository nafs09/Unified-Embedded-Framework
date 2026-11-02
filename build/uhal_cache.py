from pathlib import Path
root = Path(__file__).resolve().parents[1]
(root / 'UETD/src/uhal/arch/cortex_m/cache.c').write_text('''/// @brief Cache maintenance for Cortex-M devices that actually expose caches.
#include "cmsis.h"
#include <uhal/cache.h>
#include <ucore/assert.h>
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
void uhal_cache_clean(const void* address, uetd_u32_t size) {
    if (size == 0u) { return; }
    UETD_ASSERT(valid_range(address, size));
    if (!valid_range(address, size)) { return; }
    SCB_CleanDCache_by_Addr((void*)address, (int32_t)size); __DSB();
}
void uhal_cache_invalidate(void* address, uetd_u32_t size) {
    if (size == 0u) { return; }
    UETD_ASSERT(valid_range(address, size));
    if (!valid_range(address, size)) { return; }
    SCB_InvalidateDCache_by_Addr(address, (int32_t)size); __DSB(); __ISB();
}
void uhal_cache_clean_invalidate(void* address, uetd_u32_t size) {
    if (size == 0u) { return; }
    UETD_ASSERT(valid_range(address, size));
    if (!valid_range(address, size)) { return; }
    SCB_CleanInvalidateDCache_by_Addr(address, (int32_t)size); __DSB(); __ISB();
}
void uhal_dcache_enable(void) { SCB_EnableDCache(); __DSB(); __ISB(); }
#endif
#if UHAL_HAS_ICACHE
void uhal_icache_enable(void) { SCB_EnableICache(); __DSB(); __ISB(); }
#endif
''', encoding='utf-8', newline='\n')
p = root / 'UETD/include/uhal/cache.h'
s = p.read_text(encoding='utf-8').replace('/// @brief M7 only. No-ops on all other targets.', '/// @brief Capability-gated cache maintenance; placement belongs to the platform.')
s = s.replace('/* Enable / disable caches (called in SystemInit only) */', '/* Startup only, after memory attributes and region ownership are established. */')
s = s.replace('/// Enable the instruction cache during platform startup.\nvoid uhal_icache_enable(void);\n', '')
s = s.replace('#else /* non-M7 targets: no-ops */', '#else /* no data cache: no maintenance required */')
s = s.replace('#define uhal_cache_clean(a, s) ((void)0)', '#define uhal_cache_clean(a, s) ((void)(a), (void)(s))')
s = s.replace('#define uhal_cache_invalidate(a, s) ((void)0)', '#define uhal_cache_invalidate(a, s) ((void)(a), (void)(s))')
s = s.replace('#define uhal_cache_clean_invalidate(a, s) ((void)0)', '#define uhal_cache_clean_invalidate(a, s) ((void)(a), (void)(s))')
s = s.replace('#define uhal_icache_enable() ((void)0)\n', '')
s = s.replace('/* Memory placement attributes */', '#if UHAL_HAS_ICACHE\nvoid uhal_icache_enable(void);\n#else\n#define uhal_icache_enable() ((void)0)\n#endif\n\n/* Default placement attributes may be overridden by a platform/linker profile. */')
s = s.replace('#define UHAL_ATTR_DTCM', '#ifndef UHAL_ATTR_DTCM\n#define UHAL_ATTR_DTCM').replace('#define UHAL_ATTR_ITCM', '#endif\n#ifndef UHAL_ATTR_ITCM\n#define UHAL_ATTR_ITCM').replace('/* Non-cacheable DMA buffer', '#endif\n/* DMA buffer placement alone does not establish non-cacheability. */\n/* Non-cacheable DMA buffer')
s = s.replace('#define UHAL_ATTR_DMA_BUF UETD_SECTION(".dma_buffers") UETD_ALIGNED(32)', '#ifndef UHAL_ATTR_DMA_BUF\n#define UHAL_ATTR_DMA_BUF UETD_SECTION(".dma_buffers") UETD_ALIGNED(32)\n#endif')
p.write_text(s, encoding='utf-8', newline='\n')
print('Implemented cache operations and separated I-cache from D-cache capabilities.')
