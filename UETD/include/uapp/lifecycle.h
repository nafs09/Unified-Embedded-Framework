/// @file include/uapp/lifecycle.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UAPP_LIFECYCLE_H
#define UAPP_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

typedef enum {
    UAPP_STATE_CREATED = 0,
    UAPP_STATE_INITIALISED = 1,
    UAPP_STATE_RUNNING = 2,
    UAPP_STATE_STOPPING = 3,
    UAPP_STATE_FAULTED = 4,
    UAPP_STATE_INITIALISING = 5,
    UAPP_STATE_STARTING = 6,
} uapp_state_t;

typedef struct {
    uapp_state_t state;
    const char* name;
    uetd_status_t (*init)(void* ctx);
    uetd_status_t (*start)(void* ctx);
    uetd_status_t (*stop)(void* ctx);
    void (*on_fault)(void* ctx, uetd_u32_t fault_code);
    void* ctx;
    uetd_u32_t fault_code;
} uapp_component_t;

/// Lifecycle operations are single-owner and non-reentrant. Callback failure
/// latches FAULTED; callbacks may report a fault, which is never overwritten by
/// a successful return from the operation currently in progress.
uetd_status_t uapp_component_init(uapp_component_t* c);
uetd_status_t uapp_component_start(uapp_component_t* c);
uetd_status_t uapp_component_stop(uapp_component_t* c);
void uapp_component_fault(uapp_component_t* c, uetd_u32_t code);
uapp_state_t uapp_component_state(const uapp_component_t* c);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_LIFECYCLE_H */
