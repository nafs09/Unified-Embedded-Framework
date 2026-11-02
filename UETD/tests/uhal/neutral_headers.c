/* This compiles with a Cortex-M target macro and an absent device header:
 * public UHAL/UPAL contracts must not import the vendor's CMSIS types. */
#include <uhal/uhal.h>
#include <upal/upal.h>
int main(void) {
    uhal_irq_t irq = UHAL_IRQ_INVALID;
    uhal_fault_context_t context = {0};
    return irq == -1 && context.pc == 0u ? 0 : 1;
}
