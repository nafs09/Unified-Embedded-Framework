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
    /* TODO(HardFault_Handler):
 * 1) Capture EXC_RETURN and MSP/PSP stack frame before changing context
 * 2) snapshot fault status/address registers and preserve raw values
 * 3) store a bounded crash record then call only the configured fatal hook or safe
 *     *    reset.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_HARD);
    for (;;) { }
}

void MemManage_Handler(void) {
    /* TODO(MemManage_Handler):
 * 1) Capture EXC_RETURN and selected stack frame
 * 2) read MMFAR only when valid and record MMFSR before clearing sticky flags
 * 3) apply configured fatal/recovery policy without unsafe return.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_MEM);
    for (;;) { }
}

void BusFault_Handler(void) {
    /* TODO(BusFault_Handler):
 * 1) Capture EXC_RETURN and selected stack frame
 * 2) read BFAR only when valid and record BFSR before clearing flags
 * 3) distinguish precise/imprecise faults and use configured fatal policy.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_BUS);
    for (;;) { }
}

void UsageFault_Handler(void) {
    /* TODO(UsageFault_Handler):
 * 1) Capture EXC_RETURN and stacked frame
 * 2) snapshot UFSR causes before clearing sticky flags
 * 3) record invalid-state/PC/alignment/divide faults and route through fatal policy.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
    uhal_fault_hook((const uhal_fault_context_t*)0, UHAL_FAULT_USAGE);
    for (;;) { }
}
