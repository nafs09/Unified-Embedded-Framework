/// @file src/uhal/x86/fault.c
/// @brief Empty default fault hook for a host simulation.

#include "uef/uhal/uhal_fault.h"

/* Host processes have no MCU fault frame; this weak hook allows a test to supply fault behavior. */
/* TODO(host-fault-model): define simulated fault injection and captured context fields for tests
 * that need to exercise the UAPP fault path without relying on an actual process crash. */
UEF_WEAK void uhal_fault_hook(const uhal_fault_context_t* context,
                              uef_u32_t fault_type) {
    (void)context;
    (void)fault_type;
}
