/// @file src/uapp/lifecycle.c
/// @brief One component's explicit, guarded lifecycle state transitions.

#include "uapp/lifecycle.h"

uetd_status_t uapp_component_init(uapp_component_t* component) {
    uetd_status_t status;
    if (component == NULL || component->init == NULL) { return UETD_INVALID_ARG; }
    if (component->state != UAPP_STATE_CREATED) { return UETD_CONFLICT; }

    component->state = UAPP_STATE_INITIALISING;
    status = component->init(component->ctx);
    if (component->state == UAPP_STATE_FAULTED) { return status == UETD_OK ? UETD_ERROR : status; }
    if (status != UETD_OK) {
        uapp_component_fault(component, (uetd_u32_t)status);
        return status;
    }
    if (component->state != UAPP_STATE_INITIALISING) { return UETD_CONFLICT; }
    component->state = UAPP_STATE_INITIALISED;
    return UETD_OK;
}

uetd_status_t uapp_component_start(uapp_component_t* component) {
    uetd_status_t status;
    if (component == NULL || component->start == NULL) { return UETD_INVALID_ARG; }
    if (component->state != UAPP_STATE_INITIALISED) { return UETD_NOT_READY; }

    component->state = UAPP_STATE_STARTING;
    status = component->start(component->ctx);
    if (component->state == UAPP_STATE_FAULTED) { return status == UETD_OK ? UETD_ERROR : status; }
    if (status != UETD_OK) {
        uapp_component_fault(component, (uetd_u32_t)status);
        return status;
    }
    if (component->state != UAPP_STATE_STARTING) { return UETD_CONFLICT; }
    component->state = UAPP_STATE_RUNNING;
    return UETD_OK;
}

uetd_status_t uapp_component_stop(uapp_component_t* component) {
    uetd_status_t status = UETD_OK;
    if (component == NULL) { return UETD_INVALID_ARG; }
    if (component->state != UAPP_STATE_RUNNING) { return UETD_NOT_READY; }

    component->state = UAPP_STATE_STOPPING;
    if (component->stop != NULL) { status = component->stop(component->ctx); }
    if (component->state == UAPP_STATE_FAULTED) { return status == UETD_OK ? UETD_ERROR : status; }
    if (status != UETD_OK) {
        uapp_component_fault(component, (uetd_u32_t)status);
        return status;
    }
    if (component->state != UAPP_STATE_STOPPING) { return UETD_CONFLICT; }
    component->state = UAPP_STATE_INITIALISED;
    return UETD_OK;
}

void uapp_component_fault(uapp_component_t* component, uetd_u32_t fault_code) {
    if (component == NULL || component->state == UAPP_STATE_FAULTED) { return; }
    /* Preserve the first fault cause; recovery and subsequent diagnostics are caller-owned. */
    component->fault_code = fault_code;
    component->state = UAPP_STATE_FAULTED;
    if (component->on_fault != NULL) { component->on_fault(component->ctx, fault_code); }
}

uapp_state_t uapp_component_state(const uapp_component_t* component) {
    return component != NULL ? component->state : UAPP_STATE_FAULTED;
}
