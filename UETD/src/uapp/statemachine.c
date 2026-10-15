/// @file src/uapp/statemachine.c
/// @brief Bounded run-to-completion event queue and state transition engine.

#include "uapp/statemachine.h"
#include <string.h>

static uetd_status_t call_state_hook(uapp_statemachine_t* machine, uapp_sm_state_t state,
                                     bool entering, const uapp_sm_event_t* event) {
    uapp_sm_state_hook_fn hook;
    if (machine->state_hooks == NULL) { return UETD_OK; }
    hook = entering ? machine->state_hooks[state].on_enter : machine->state_hooks[state].on_exit;
    return hook != NULL ? hook(state, event, machine->ctx) : UETD_OK;
}

static void latch_fault(uapp_statemachine_t* machine, const uapp_sm_event_t* event, uetd_status_t status) {
    if (machine->faulted) { return; }
    machine->faulted = true;
    machine->fault_status = status;
    machine->state_active = false;
    machine->queue_head = 0u;
    machine->queue_tail = 0u;
    machine->queue_count = 0u;
    if (machine->on_fault != NULL) { machine->on_fault(machine->current, event, status, machine->ctx); }
}

uetd_status_t uapp_sm_make_event(uapp_sm_event_t* event, uetd_u8_t id,
                                 const void* payload, size_t payload_size) {
    if (event == NULL || payload_size > UAPP_SM_EVENT_PAYLOAD_CAPACITY ||
        (payload_size != 0u && payload == NULL)) { return UETD_INVALID_ARG; }
    /* memmove first so payload may safely point into the event being rebuilt. */
    if (payload_size != 0u) { memmove(event->payload, payload, payload_size); }
    event->id = id;
    event->size = (uetd_u8_t)payload_size;
    memset(event->payload + payload_size, 0, sizeof(event->payload) - payload_size);
    return UETD_OK;
}

uetd_status_t uapp_sm_init(uapp_statemachine_t* machine, const uapp_sm_config_t* config) {
    uetd_status_t status;
    if (machine == NULL) { return UETD_INVALID_ARG; }
    /* The public contract requires UAPP_SM_INITIALIZER before this first read. */
    if (machine->initialized) { return UETD_BUSY; }
    if (config == NULL || config->state_count == 0u ||
        config->state_count > UAPP_SM_MAX_STATES || config->initial_state >= config->state_count ||
        config->transition_count > UAPP_SM_MAX_TRANSITIONS ||
        (config->transition_count != 0u && config->transitions == NULL)) { return UETD_INVALID_ARG; }
    for (uetd_u16_t i = 0u; i < config->transition_count; ++i) {
        const uapp_sm_transition_t* transition = &config->transitions[i];
        if (transition->from >= config->state_count || transition->to >= config->state_count) {
            return UETD_INVALID_ARG;
        }
        /* An earlier unconditional duplicate makes this row unreachable. */
        for (uetd_u16_t j = 0u; j < i; ++j) {
            const uapp_sm_transition_t* earlier = &config->transitions[j];
            if (earlier->from == transition->from && earlier->event_id == transition->event_id &&
                earlier->guard == NULL) {
                return UETD_INVALID_ARG;
            }
        }
    }

    memset(machine, 0, sizeof(*machine));
    machine->current = config->initial_state;
    machine->initial = config->initial_state;
    machine->transitions = config->transitions;
    machine->state_hooks = config->state_hooks;
    machine->transition_count = config->transition_count;
    machine->state_count = config->state_count;
    machine->on_unhandled = config->on_unhandled;
    machine->on_fault = config->on_fault;
    machine->ctx = config->ctx;
    machine->initialized = true;
    machine->processing = true;

    status = call_state_hook(machine, machine->current, true, NULL);
    if (status == UETD_OK) {
        machine->state_active = true;
    } else {
        latch_fault(machine, NULL, status);
    }
    machine->processing = false;
    return status;
}

uetd_status_t uapp_sm_post(uapp_statemachine_t* machine, const uapp_sm_event_t* event) {
    if (machine == NULL || event == NULL || event->size > UAPP_SM_EVENT_PAYLOAD_CAPACITY) {
        return UETD_INVALID_ARG;
    }
    if (!machine->initialized || machine->faulted) { return UETD_NOT_READY; }
    if (machine->queue_count >= UAPP_SM_EVENT_QUEUE_CAPACITY) { return UETD_OVERFLOW; }
    uapp_sm_event_t* const queued = &machine->event_queue[machine->queue_head];
    queued->id = event->id;
    queued->size = event->size;
    if (event->size != 0u) { memmove(queued->payload, event->payload, event->size); }
    memset(queued->payload + event->size, 0, sizeof(queued->payload) - event->size);
    machine->queue_head = (uetd_u8_t)((machine->queue_head + 1u) % UAPP_SM_EVENT_QUEUE_CAPACITY);
    ++machine->queue_count;
    return UETD_OK;
}

static uetd_status_t process_event(uapp_statemachine_t* machine, const uapp_sm_event_t* event) {
    const uapp_sm_state_t from = machine->current;
    const uapp_sm_transition_t* selected = NULL;
    uetd_status_t status;

    for (uetd_u16_t i = 0u; i < machine->transition_count; ++i) {
        const uapp_sm_transition_t* transition = &machine->transitions[i];
        if (transition->from != from || transition->event_id != event->id) { continue; }
        if (transition->guard != NULL && !transition->guard(event, machine->ctx)) { continue; }
        selected = transition;
        break;
    }
    if (selected == NULL) {
        if (machine->on_unhandled != NULL) { machine->on_unhandled(from, event, machine->ctx); }
        return UETD_NOT_SUPPORTED;
    }

    machine->state_active = false;
    status = call_state_hook(machine, from, false, event);
    if (status != UETD_OK) { latch_fault(machine, event, status); return status; }
    if (selected->action != NULL) {
        status = selected->action(event, machine->ctx);
        if (status != UETD_OK) { latch_fault(machine, event, status); return status; }
    }

    machine->current = selected->to;
    status = call_state_hook(machine, machine->current, true, event);
    if (status != UETD_OK) { latch_fault(machine, event, status); return status; }
    machine->state_active = true;
    return UETD_OK;
}

uetd_status_t uapp_sm_process_one(uapp_statemachine_t* machine) {
    uapp_sm_event_t event;
    uetd_status_t status;
    if (machine == NULL) { return UETD_INVALID_ARG; }
    if (!machine->initialized || machine->faulted) { return UETD_NOT_READY; }
    if (machine->processing) { return UETD_BUSY; }
    if (machine->queue_count == 0u) { return UETD_NOT_READY; }

    event = machine->event_queue[machine->queue_tail];
    machine->queue_tail = (uetd_u8_t)((machine->queue_tail + 1u) % UAPP_SM_EVENT_QUEUE_CAPACITY);
    --machine->queue_count;
    machine->processing = true;
    status = process_event(machine, &event);
    machine->processing = false;
    return status;
}

uetd_status_t uapp_sm_reset(uapp_statemachine_t* machine) {
    uetd_status_t status;
    if (machine == NULL) { return UETD_INVALID_ARG; }
    if (!machine->initialized) { return UETD_NOT_READY; }
    if (machine->processing) { return UETD_BUSY; }

    machine->processing = true;
    machine->queue_head = 0u;
    machine->queue_tail = 0u;
    machine->queue_count = 0u;
    if (machine->state_active) {
        machine->state_active = false;
        status = call_state_hook(machine, machine->current, false, NULL);
        if (status != UETD_OK) {
            latch_fault(machine, NULL, status);
            machine->processing = false;
            return status;
        }
    }

    machine->current = machine->initial;
    machine->faulted = false;
    machine->fault_status = UETD_OK;
    status = call_state_hook(machine, machine->current, true, NULL);
    if (status == UETD_OK) {
        machine->state_active = true;
    } else {
        latch_fault(machine, NULL, status);
    }
    machine->processing = false;
    return status;
}

uetd_status_t uapp_sm_get_state(const uapp_statemachine_t* machine, uapp_sm_state_t* state) {
    if (machine == NULL || state == NULL) { return UETD_INVALID_ARG; }
    if (!machine->initialized) { return UETD_NOT_READY; }
    *state = machine->current;
    return UETD_OK;
}

bool uapp_sm_is_faulted(const uapp_statemachine_t* machine) {
    return machine != NULL && machine->initialized && machine->faulted;
}

bool uapp_sm_is_active(const uapp_statemachine_t* machine) {
    return machine != NULL && machine->initialized && machine->state_active && !machine->faulted;
}

uetd_status_t uapp_sm_get_fault_status(const uapp_statemachine_t* machine, uetd_status_t* status) {
    if (machine == NULL || status == NULL) { return UETD_INVALID_ARG; }
    if (!machine->initialized || !machine->faulted) { return UETD_NOT_READY; }
    *status = machine->fault_status;
    return UETD_OK;
}
