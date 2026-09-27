/// @file src/uapp/lifecycle.c
/// @brief One component's small, explicit lifecycle state machine.

#include "uef/uapp/uapp_lifecycle.h"

uef_status_t uapp_component_init(uapp_component_t* component) {
    if (component == NULL || component->init == NULL) {
        return UEF_INVALID_ARG;
    }
    if (component->state != UAPP_STATE_CREATED) {
        return UEF_CONFLICT;
    }

    /* State changes only after the callback succeeds; a failed callback enters FAULTED below. */
    const uef_status_t status = component->init(component->ctx);
    if (status == UEF_OK) {
        component->state = UAPP_STATE_INITIALISED;
    } else {
        uapp_component_fault(component, (uef_u32_t)status);
    }
    return status;
}

uef_status_t uapp_component_start(uapp_component_t* component) {
    if (component == NULL || component->start == NULL) {
        return UEF_INVALID_ARG;
    }
    if (component->state != UAPP_STATE_INITIALISED) {
        return UEF_NOT_READY;
    }

    /* Starting is legal only after initialization; do not implicitly rerun init here. */
    const uef_status_t status = component->start(component->ctx);
    if (status == UEF_OK) {
        component->state = UAPP_STATE_RUNNING;
    } else {
        uapp_component_fault(component, (uef_u32_t)status);
    }
    return status;
}

void uapp_component_stop(uapp_component_t* component) {
    if (component == NULL || component->state != UAPP_STATE_RUNNING) {
        return;
    }

    /* Publish STOPPING before user code runs so observers do not mistake teardown for RUNNING. */
    component->state = UAPP_STATE_STOPPING;
    if (component->stop != NULL) {
        component->stop(component->ctx);
    }
    component->state = UAPP_STATE_INITIALISED;
}

void uapp_component_fault(uapp_component_t* component, uef_u32_t fault_code) {
    if (component == NULL) {
        return;
    }
    /* Fault notification is intentionally terminal here; recovery policy belongs to the caller. */
    component->state = UAPP_STATE_FAULTED;
    if (component->on_fault != NULL) {
        component->on_fault(component->ctx, fault_code);
    }
}

uapp_state_t uapp_component_state(const uapp_component_t* component) {
    return component != NULL ? component->state : UAPP_STATE_FAULTED;
}
