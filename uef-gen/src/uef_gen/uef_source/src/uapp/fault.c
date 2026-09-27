/// @file src/uapp/fault.c
/// @brief Persist minimal fault metadata and enter the platform reset path.

#include "uef/uapp/uapp_fault.h"
#include "uef/ucore/uef_status.h"
#include "uef/uhal/uhal_memory.h"
#include "uef/uhal/uhal_core.h"

#include <stdlib.h>

void uapp_fault_report(uef_u32_t fault_code, const char* detail) {
    /* Application recovery/logging policy is supplied by the product layer. */
    (void)fault_code;
    (void)detail;
}

UEF_NORETURN void uapp_fault_fatal(uef_u32_t fault_code, const char* detail) {
    /* Detail text is deliberately not persisted in the small backup register set. */
    (void)detail;
    uhal_backup_write(UHAL_BACKUP_FAULT_CODE, fault_code);
    uhal_backup_write(UHAL_BACKUP_FAULT_PC, 0u);
    uhal_backup_write(UHAL_BACKUP_FAULT_LR, 0u);
    uhal_backup_write(UHAL_BACKUP_RESET_CAUSE, UHAL_RESET_FAULT);
    uhal_system_reset();

    /* A real target reset does not return. Keep host/debug fallback fail-fast. */
    abort();
}

bool uapp_fault_log_available(void) {
    return uhal_backup_read(UHAL_BACKUP_FAULT_CODE) != 0u;
}

uef_u32_t uapp_fault_log_code(void) {
    return uhal_backup_read(UHAL_BACKUP_FAULT_CODE);
}

uef_u32_t uapp_fault_log_pc(void) {
    return uhal_backup_read(UHAL_BACKUP_FAULT_PC);
}

uef_u32_t uapp_fault_log_lr(void) {
    return uhal_backup_read(UHAL_BACKUP_FAULT_LR);
}
