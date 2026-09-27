/// @file src/uhal/arm_cm/fault.c
/// @brief Cortex-M fatal-exception entry points and the weak application fault hook.
///
/// The exception-frame decoder and persistent crash record are board-dependent.
/// Until those pieces are implemented, these handlers fail-stop and never resume
/// execution at the faulting instruction.
#include <uef/uhal/uhal_fault.h>

/* TODO(UEF Cortex-M fault handling):
 * Decode the exception stack frame selected by EXC_RETURN, including optional
 * floating-point stacking. Copy captured registers into uhal_fault_context_t,
 * persist the fault before attempting reset, and call the application hook with
 * the correct class. Keep this path allocation-free and guarantee that handlers
 * never return to the fault site. The loops below are the safe fallback until
 * persistent fault storage and board reset policy are implemented.
 */
UEF_WEAK void uhal_fault_hook(const uhal_fault_context_t* context,
                              uef_u32_t fault_type) {
    (void)context;
    (void)fault_type;
    for (;;) {
        /* Halt rather than continue after an unhandled fatal fault. */
    }
}

void HardFault_Handler(void) {
    /* TODO: Capture the exception stack frame and fault status registers. */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_HARD);
    for (;;) { }
}

void MemManage_Handler(void) {
    /* TODO: Capture the exception stack frame, MMFAR, and memory-fault status. */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_MEM);
    for (;;) { }
}

void BusFault_Handler(void) {
    /* TODO: Capture the exception stack frame, BFAR, and bus-fault status. */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_BUS);
    for (;;) { }
}

void UsageFault_Handler(void) {
    /* TODO: Capture the exception stack frame and usage-fault status. */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_USAGE);
    for (;;) { }
}
