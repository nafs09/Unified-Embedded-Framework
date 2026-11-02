#include "mock_cmsis.h"
#include <uhal/uhal.h>
#include <stdio.h>
#define CHECK(c) do { if (!(c)) { printf("Failed line %d: %s\n", __LINE__, #c); return 1; } } while (0)
uint32_t mock_mask, mock_basepri, mock_enabled, mock_pending, mock_priority;
uint32_t mock_clean, mock_invalidate, mock_icache, mock_dcache;
MockDWT mock_dwt;
uetd_u32_t uhal_cpu_freq_hz(void) { return 48000000u; }
uetd_u64_t uhal_cycle_count_64(void) { return 0u; }
int main(void) {
    uhal_critical_t outer = uhal_critical_enter();
    CHECK(outer == 0u && mock_mask == 1u);
    uhal_critical_t inner = uhal_critical_enter();
    uhal_critical_exit(inner); CHECK(mock_mask == 1u);
    uhal_critical_exit(outer); CHECK(mock_mask == 0u);
    mock_mask = 1u; outer = uhal_critical_enter(); uhal_critical_exit(outer); CHECK(mock_mask == 1u);
    volatile uetd_u32_t value = 5u;
    CHECK(uhal_atomic_fetch_add_u32(&value, 7u) == 5u && value == 12u && mock_mask == 1u);
    CHECK(uhal_atomic_cas_u32(&value, 12u, 9u) && value == 9u);
    CHECK(!uhal_atomic_cas_u32(&value, 12u, 4u) && value == 9u);
    CHECK(uhal_irq_enable(-1, 0u, 0u) == UETD_INVALID_ARG);
    CHECK(uhal_irq_enable(32, 0u, 0u) == UETD_INVALID_ARG);
    CHECK(uhal_irq_enable(3, 16u, 0u) == UETD_INVALID_ARG && mock_enabled == 0u);
    CHECK(uhal_irq_enable(3, 2u, 1u) == UETD_INVALID_ARG && mock_enabled == 0u);
    CHECK(uhal_irq_enable(3, 2u, 0u) == UETD_OK && mock_enabled == 8u && mock_priority == 2u);
    CHECK(uhal_irq_set_pending(3) == UETD_OK && mock_pending == 8u);
    CHECK(uhal_irq_clear_pending(3) == UETD_OK && mock_pending == 0u);
    CHECK(uhal_irq_disable(3) == UETD_OK && mock_enabled == 0u);
    CHECK(uhal_core_id() == 0u && uhal_cycles_per_us() == 48u);
#if UHAL_HAS_DWT
    mock_dwt.CYCCNT = 456u;
    CHECK(uhal_cycle_count() == 0u);
    mock_dwt.CTRL = 1u; CHECK(uhal_cycle_count() == 456u);
#else
    CHECK(uhal_cycle_count() == 0u);
#endif
#if UHAL_HAS_IRQ_PRIORITY_MASK
    mock_basepri = 96u;
    outer = uhal_basepri_enter(8u); CHECK(mock_basepri == 96u);
    inner = uhal_basepri_enter(2u); CHECK(mock_basepri == 32u);
    uhal_basepri_exit(inner); CHECK(mock_basepri == 96u);
    uhal_basepri_exit(outer); CHECK(mock_basepri == 96u);
#endif
#if UHAL_HAS_DCACHE
    UETD_ALIGNAS(32) unsigned char buffer[64];
    CHECK(uhal_cache_clean(buffer, sizeof(buffer)) == UETD_OK && mock_clean == 64u);
    CHECK(uhal_cache_invalidate(buffer + 1, 32u) == UETD_CACHE_ERROR && mock_invalidate == 0u);
    CHECK(uhal_cache_invalidate(buffer, 31u) == UETD_CACHE_ERROR && mock_invalidate == 0u);
    uhal_cache_invalidate(buffer, 64u); CHECK(mock_invalidate == 64u);
    uhal_cache_clean_invalidate(buffer, 32u); CHECK(mock_clean == 96u && mock_invalidate == 96u);
    uhal_dcache_enable(); uhal_icache_enable(); CHECK(mock_dcache == 1u && mock_icache == 1u);
#endif
    return 0;
}
