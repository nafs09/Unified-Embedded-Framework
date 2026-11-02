/// @file src/uhal/platform/host/fault.c
/// @brief An unhandled fatal simulation fault terminates the process.

#include "uhal/fault.h"
#include <stdlib.h>

/* Host processes have no MCU fault frame; this weak hook allows a test to supply fault behavior. */
/* TODO(host-fault-model): define simulated fault injection and captured context fields for tests
 * that need to exercise the
 * UAPP fault path without relying on an actual process crash. */
UETD_WEAK void uhal_fault_hook(const uhal_fault_context_t* context, uetd_u32_t fault_type) {
    (void)context;
    (void)fault_type;
    abort();
}
