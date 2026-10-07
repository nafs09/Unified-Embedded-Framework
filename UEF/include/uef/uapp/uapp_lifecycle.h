/// @file include/uef/uapp/uapp_lifecycle.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UAPP_LIFECYCLE_H
#define UAPP_LIFECYCLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

typedef enum {
    UAPP_STATE_CREATED      = 0,
    UAPP_STATE_INITIALISED  = 1,
    UAPP_STATE_RUNNING      = 2,
    UAPP_STATE_STOPPING     = 3,
    UAPP_STATE_FAULTED      = 4,
} uapp_state_t;

typedef struct {
    uapp_state_t        state;
    const char*         name;
    uef_status_t (*init)(void* ctx);
    uef_status_t (*start)(void* ctx);
    void         (*stop)(void* ctx);
    void         (*on_fault)(void* ctx, uef_u32_t fault_code);
    void*        ctx;
} uapp_component_t;

uef_status_t uapp_component_init(uapp_component_t* c);
uef_status_t uapp_component_start(uapp_component_t* c);
void         uapp_component_stop(uapp_component_t* c);
void         uapp_component_fault(uapp_component_t* c, uef_u32_t code);
uapp_state_t uapp_component_state(const uapp_component_t* c);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_LIFECYCLE_H */
