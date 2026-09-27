/// @file include/uef/uhal/uhal_cache.h
/// @brief M7 only. No-ops on all other targets.

#ifndef UHAL_CACHE_H
#define UHAL_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/uhal/target.h"

#if UHAL_HAS_DCACHE

/* Clean (CPU→SRAM): call before DMA reads from a cacheable buffer */
void uhal_cache_clean(const void* addr, uef_u32_t size);

/* Invalidate (SRAM→CPU): call after DMA writes to a cacheable buffer */
void uhal_cache_invalidate(void* addr, uef_u32_t size);

/* Clean + Invalidate: for read-write DMA regions */
void uhal_cache_clean_invalidate(void* addr, uef_u32_t size);

/* Enable / disable caches (called in SystemInit only) */
void uhal_dcache_enable(void);
void uhal_icache_enable(void);

#else   /* non-M7 targets: no-ops */
#  define uhal_cache_clean(a,s)            ((void)0)
#  define uhal_cache_invalidate(a,s)       ((void)0)
#  define uhal_cache_clean_invalidate(a,s) ((void)0)
#  define uhal_dcache_enable()             ((void)0)
#  define uhal_icache_enable()             ((void)0)
#endif

/* Memory placement attributes */
#define UHAL_ATTR_DTCM      UEF_SECTION(".dtcm_data")
#define UHAL_ATTR_ITCM      UEF_SECTION(".itcm_text")
/* Non-cacheable DMA buffer — coherent without maintenance */
#define UHAL_ATTR_DMA_BUF   UEF_SECTION(".dma_buffers") UEF_ALIGNED(32)

#ifdef __cplusplus
}
#endif

#endif /* UHAL_CACHE_H */
