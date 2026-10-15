/// @file include/uhal/fault.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UHAL_FAULT_H
#define UHAL_FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"
#include "uhal/target.h"

typedef struct {
    uetd_u32_t r0, r1, r2, r3, r12;
    uetd_u32_t lr; /* link register at fault */
    uetd_u32_t pc; /* faulting address */
    uetd_u32_t psr;
    /* FPU context if active at fault time */
    uetd_f32_t s[16];
    uetd_u32_t fpscr;
} uhal_fault_context_t;

#define UHAL_FAULT_HARD (1u << 0)
#define UHAL_FAULT_MEM (1u << 1)
#define UHAL_FAULT_BUS (1u << 2)
#define UHAL_FAULT_USAGE (1u << 3)

/* Application-provided hook. Called from framework HardFault/MemFault handlers.
 * Must not return. Store info to RTC backup registers and trigger reset. */
UETD_WEAK
void uhal_fault_hook(const uhal_fault_context_t* ctx, uetd_u32_t fault_type);

/* Default handler implementations — call uhal_fault_hook then reset */
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* UHAL_FAULT_H */
