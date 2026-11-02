/* Implement these callbacks in application-owned C files. Never edit generated tables. */
#ifndef MANUAL_RUNTIME_CALLBACKS_H
#define MANUAL_RUNTIME_CALLBACKS_H
#include "runtime_config.h"
#include "uos/uos.h"

uetd_status_t motor_enter(uapp_sm_state_t state, const uapp_sm_event_t* event, void* context);

uetd_status_t motor_exit(uapp_sm_state_t state, const uapp_sm_event_t* event, void* context);

bool start_allowed(const uapp_sm_event_t* event, void* context);

void controller_failure(uapp_sm_state_t state, const uapp_sm_event_t* event, uetd_status_t status, void* context);

#endif
