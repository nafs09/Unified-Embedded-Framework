/* Generated from explicit manual input. One task/main-loop owns each instance. */
#ifndef MANUAL_MINIMAL_H
#define MANUAL_MINIMAL_H
#include "runtime_config.h"
typedef struct { uapp_statemachine_t machine; } minimal_t;
#define MINIMAL_INITIALIZER { UAPP_SM_INITIALIZER }
enum {
    MINIMAL_STATE_IDLE = 0
};

uetd_status_t minimal_init(minimal_t* instance, void* context);
uetd_status_t minimal_post(minimal_t* instance, uetd_u8_t event_id, const void* payload, size_t size);
uetd_status_t minimal_process(minimal_t* instance);
uetd_status_t minimal_reset(minimal_t* instance);
uetd_status_t minimal_state(const minimal_t* instance, uapp_sm_state_t* state);
#endif
