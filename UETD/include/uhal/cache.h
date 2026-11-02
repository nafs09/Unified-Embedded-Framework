/// @file include/uhal/cache.h
/// @brief Capability-gated cache maintenance; placement belongs to the platform.

#ifndef UHAL_CACHE_H
#define UHAL_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"
#include "ucore/status.h"
#include "uhal/target.h"

#if UHAL_HAS_DCACHE
#if UHAL_CACHE_LINE_SIZE == 0
#error "A data-cache platform must declare UHAL_CACHE_LINE_SIZE"
#endif

/* Clean (CPU→SRAM): call before DMA reads from a cacheable buffer */
/// Clean cache lines so DMA can read the latest CPU-written buffer data.
uetd_status_t uhal_cache_clean(const void* addr, uetd_u32_t size);

/* Invalidate (SRAM→CPU): call after DMA writes to a cacheable buffer */
/// Invalidate cache lines after DMA writes to the buffer.
uetd_status_t uhal_cache_invalidate(void* addr, uetd_u32_t size);

/* Clean + Invalidate: for read-write DMA regions */
/// Clean and invalidate cache lines for a DMA read-write buffer.
uetd_status_t uhal_cache_clean_invalidate(void* addr, uetd_u32_t size);

/* Startup only, after memory attributes and region ownership are established. */
/// Enable the data cache during platform startup.
void uhal_dcache_enable(void);

#else /* no data cache: no maintenance required */
static inline uetd_status_t uhal_cache_clean(const void* address, uetd_u32_t size) { (void)address; (void)size; return UETD_OK; }
static inline uetd_status_t uhal_cache_invalidate(void* address, uetd_u32_t size) { (void)address; (void)size; return UETD_OK; }
static inline uetd_status_t uhal_cache_clean_invalidate(void* address, uetd_u32_t size) { (void)address; (void)size; return UETD_OK; }
#define uhal_dcache_enable() ((void)0)
#endif

#if UHAL_HAS_ICACHE
void uhal_icache_enable(void);
#else
#define uhal_icache_enable() ((void)0)
#endif

/* Default placement attributes may be overridden by a platform/linker profile. */
#ifndef UHAL_ATTR_DTCM
#define UHAL_ATTR_DTCM UETD_SECTION(".dtcm_data")
#endif
#ifndef UHAL_ATTR_ITCM
#define UHAL_ATTR_ITCM UETD_SECTION(".itcm_text")
#endif
/* DMA buffer placement alone does not establish non-cacheability. */
/* The memory/MPU profile decides whether maintenance is needed for this section. */
#ifndef UHAL_ATTR_DMA_BUF
#define UHAL_ATTR_DMA_BUF UETD_SECTION(".dma_buffers") UETD_ALIGNED(32)
#endif

#ifdef __cplusplus
}
#endif

#endif /* UHAL_CACHE_H */
