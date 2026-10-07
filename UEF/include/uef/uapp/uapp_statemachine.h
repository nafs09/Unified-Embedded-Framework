/// @file include/uef/uapp/uapp_statemachine.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UAPP_STATEMACHINE_H
#define UAPP_STATEMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

#define UAPP_SM_MAX_STATES       16
#define UAPP_SM_MAX_TRANSITIONS  32

typedef uef_u8_t uapp_sm_state_t;
typedef uef_u8_t uapp_sm_event_t;

typedef struct {
    uapp_sm_state_t  from;
    uapp_sm_event_t  event;
    uapp_sm_state_t  to;
    void (*action)(void* ctx);
    bool (*guard)(void* ctx);
} uapp_sm_transition_t;

typedef struct {
    uapp_sm_state_t       current;
    const uapp_sm_transition_t* transitions;
    uef_u8_t              n_transitions;
    void (*on_enter[UAPP_SM_MAX_STATES])(void* ctx);
    void (*on_exit[UAPP_SM_MAX_STATES])(void* ctx);
    void* ctx;
} uapp_statemachine_t;

void uapp_sm_init(uapp_statemachine_t* sm,
                   const uapp_sm_transition_t* transitions,
                   uef_u8_t n_transitions,
                   uapp_sm_state_t initial, void* ctx);
bool uapp_sm_dispatch(uapp_statemachine_t* sm, uapp_sm_event_t event);
uapp_sm_state_t uapp_sm_state(const uapp_statemachine_t* sm);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_STATEMACHINE_H */
