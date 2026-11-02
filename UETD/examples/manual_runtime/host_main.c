/* Deterministic host example; does not control physical outputs. */
#include "controller.h"
#include "callbacks.h"
#include <stdio.h>
typedef struct { bool enabled, permitted; unsigned faults; } application_t;
bool start_allowed(const uapp_sm_event_t* event, void* context) {
    (void)event; return ((application_t*)context)->permitted;
}
uetd_status_t motor_enter(uapp_sm_state_t state, const uapp_sm_event_t* event, void* context) {
    (void)state; (void)event; ((application_t*)context)->enabled = true; return UETD_OK;
}
uetd_status_t motor_exit(uapp_sm_state_t state, const uapp_sm_event_t* event, void* context) {
    (void)state; (void)event; ((application_t*)context)->enabled = false; return UETD_OK;
}
void controller_failure(uapp_sm_state_t state, const uapp_sm_event_t* event, uetd_status_t error, void* context) {
    (void)state; (void)event; (void)error;
    application_t* app = context; app->enabled = false; ++app->faults;
}
int main(void) {
    controller_t controller = CONTROLLER_INITIALIZER;
    application_t app = {false, true, 0};
    if (uos_clock_use_platform() != UOS_OK || controller_init(&controller, &app) != UETD_OK) { return 1; }
    if (controller_post(&controller, CONTROLLER_EVENT_START, NULL, 0) != UETD_OK ||
        controller_process(&controller) != UETD_OK || !app.enabled) { return 2; }
    if (controller_post(&controller, CONTROLLER_EVENT_STOP, NULL, 0) != UETD_OK ||
        controller_process(&controller) != UETD_OK || app.enabled || app.faults) { return 3; }
    puts("Manual UOS/UAPP template example passed"); return 0;
}
