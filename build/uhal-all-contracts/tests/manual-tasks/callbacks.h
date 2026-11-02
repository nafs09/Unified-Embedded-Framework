/* Implement these callbacks in application-owned C files. Never edit generated tables. */
#ifndef MANUAL_RUNTIME_CALLBACKS_H
#define MANUAL_RUNTIME_CALLBACKS_H
#include "runtime_config.h"
#include "uos/uos.h"

uetd_status_t owner_step(void* context);

/* These handlers must establish the product's safe state and not return. */
UETD_NORETURN void manual_runtime_os_failure(uos_status_t status);
UETD_NORETURN void manual_runtime_component_failure(uetd_status_t status);
#endif
