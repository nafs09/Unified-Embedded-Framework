#ifndef UETD_MOCK_CMSIS_H
#define UETD_MOCK_CMSIS_H
#include <stdint.h>
#define __CORTEX_M MOCK_CORE
#define __NVIC_PRIO_BITS 4u
typedef int32_t IRQn_Type;
extern uint32_t mock_mask, mock_basepri, mock_enabled, mock_pending, mock_priority;
extern uint32_t mock_clean, mock_invalidate, mock_icache, mock_dcache;
typedef struct { uint32_t CTRL, CYCCNT; } MockDWT;
extern MockDWT mock_dwt;
#define DWT (&mock_dwt)
#define DWT_CTRL_CYCCNTENA_Msk 1u
static inline uint32_t __get_PRIMASK(void) { return mock_mask; }
static inline void __disable_irq(void) { mock_mask = 1u; }
static inline void __set_PRIMASK(uint32_t mask) { mock_mask = mask; }
static inline void __DMB(void) {}
static inline void __DSB(void) {}
static inline void __ISB(void) {}
static inline void __NOP(void) {}
#if MOCK_CORE != 0 && MOCK_CORE != 1 && MOCK_CORE != 23
static inline uint32_t __get_BASEPRI(void) { return mock_basepri; }
static inline void __set_BASEPRI(uint32_t value) { mock_basepri = value; }
static inline void __set_BASEPRI_MAX(uint32_t value) {
    if (value && (!mock_basepri || value < mock_basepri)) { mock_basepri = value; }
}
static inline uint32_t NVIC_GetPriorityGrouping(void) { return 0u; }
static inline uint32_t NVIC_EncodePriority(uint32_t group, uint32_t p, uint32_t s) { (void)group; (void)s; return p & 15u; }
static inline void NVIC_DecodePriority(uint32_t p, uint32_t group, uint32_t* priority, uint32_t* sub) { (void)group; *priority = p; *sub = 0u; }
#endif
static inline void NVIC_SetPriority(IRQn_Type irq, uint32_t value) { (void)irq; mock_priority = value; }
static inline void NVIC_EnableIRQ(IRQn_Type irq) { mock_enabled |= UINT32_C(1) << irq; }
static inline void NVIC_DisableIRQ(IRQn_Type irq) { mock_enabled &= ~(UINT32_C(1) << irq); }
static inline void NVIC_SetPendingIRQ(IRQn_Type irq) { mock_pending |= UINT32_C(1) << irq; }
static inline void NVIC_ClearPendingIRQ(IRQn_Type irq) { mock_pending &= ~(UINT32_C(1) << irq); }
static inline void NVIC_SystemReset(void) {}
static inline void SCB_CleanDCache_by_Addr(void* p, int32_t size) { (void)p; mock_clean += (uint32_t)size; }
static inline void SCB_InvalidateDCache_by_Addr(void* p, int32_t size) { (void)p; mock_invalidate += (uint32_t)size; }
static inline void SCB_CleanInvalidateDCache_by_Addr(void* p, int32_t size) { SCB_CleanDCache_by_Addr(p, size); SCB_InvalidateDCache_by_Addr(p, size); }
static inline void SCB_EnableDCache(void) { ++mock_dcache; }
static inline void SCB_EnableICache(void) { ++mock_icache; }
#endif
