/// @file src/uapp/statemachine.c
/// @brief Table-driven state machine with bounded transition lookup.

#include "uef/uapp/uapp_statemachine.h"

#include <string.h>

void uapp_sm_init(uapp_statemachine_t* machine,
                  const uapp_sm_transition_t* transitions,
                  uef_u8_t transition_count,
                  uapp_sm_state_t initial_state,
                  void* context) {
    if (machine == NULL) {
        return;
    }

    /* Clear callback tables as well as runtime fields so reused storage has no stale hooks. */
    memset(machine, 0, sizeof(*machine));
    machine->current = initial_state;
    machine->ctx = context;
    if (transitions != NULL && transition_count <= UAPP_SM_MAX_TRANSITIONS) {
        machine->transitions = transitions;
        machine->n_transitions = transition_count;
    }
}

bool uapp_sm_dispatch(uapp_statemachine_t* machine, uapp_sm_event_t event) {
    if (machine == NULL || machine->transitions == NULL ||
        machine->current >= UAPP_SM_MAX_STATES) {
        return false;
    }

    /* Declaration order is the tie-breaker if multiple guarded transitions match. */
    for (uef_u8_t index = 0u; index < machine->n_transitions; ++index) {
        const uapp_sm_transition_t* transition = &machine->transitions[index];
        if (transition->from != machine->current || transition->event != event ||
            transition->to >= UAPP_SM_MAX_STATES) {
            continue;
        }
        if (transition->guard != NULL && !transition->guard(machine->ctx)) {
            continue;
        }

        /* Exit old state, publish the new state, then run action and entry hook in that order. */
        const uapp_sm_state_t previous = machine->current;
        if (machine->on_exit[previous] != NULL) {
            machine->on_exit[previous](machine->ctx);
        }
        machine->current = transition->to;
        if (transition->action != NULL) {
            transition->action(machine->ctx);
        }
        if (machine->on_enter[machine->current] != NULL) {
            machine->on_enter[machine->current](machine->ctx);
        }
        return true;
    }
    return false;
}

uapp_sm_state_t uapp_sm_state(const uapp_statemachine_t* machine) {
    return machine != NULL ? machine->current : 0u;
}
