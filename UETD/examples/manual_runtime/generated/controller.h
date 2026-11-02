/* Generated from explicit manual input. One task/main-loop owns each instance. */
#ifndef MANUAL_CONTROLLER_H
#define MANUAL_CONTROLLER_H
#include "runtime_config.h"
typedef struct { uapp_statemachine_t machine; } controller_t;
#define CONTROLLER_INITIALIZER { UAPP_SM_INITIALIZER }
enum {
    CONTROLLER_STATE_IDLE = 0,
    CONTROLLER_STATE_RUNNING = 1
};
enum {
    CONTROLLER_EVENT_START = 1,
    CONTROLLER_EVENT_STOP = 2
};

uetd_status_t controller_init(controller_t* instance, void* context);
uetd_status_t controller_post(controller_t* instance, uetd_u8_t event_id, const void* payload, size_t size);
uetd_status_t controller_process(controller_t* instance);
uetd_status_t controller_reset(controller_t* instance);
uetd_status_t controller_state(const controller_t* instance, uapp_sm_state_t* state);
#endif
