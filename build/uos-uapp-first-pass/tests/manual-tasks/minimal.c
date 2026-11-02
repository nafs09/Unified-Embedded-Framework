#include "minimal.h"
#include "callbacks.h"
static const uapp_sm_state_hooks_t hooks[] = {
    { NULL, NULL },
};
uetd_status_t minimal_init(minimal_t* instance, void* context) {
    if (instance == NULL) { return UETD_INVALID_ARG; }
    const uapp_sm_config_t config = {
        NULL, 0, hooks, 1, 0,
        NULL, NULL, context
    };
    return uapp_sm_init(&instance->machine, &config);
}
uetd_status_t minimal_post(minimal_t* instance, uetd_u8_t id, const void* payload, size_t size) {
    if (instance == NULL) { return UETD_INVALID_ARG; }
    uapp_sm_event_t event;
    const uetd_status_t status = uapp_sm_make_event(&event, id, payload, size);
    return status == UETD_OK ? uapp_sm_post(&instance->machine, &event) : status;
}
uetd_status_t minimal_process(minimal_t* instance) {
    return instance != NULL ? uapp_sm_process_one(&instance->machine) : UETD_INVALID_ARG;
}
uetd_status_t minimal_reset(minimal_t* instance) {
    return instance != NULL ? uapp_sm_reset(&instance->machine) : UETD_INVALID_ARG;
}
uetd_status_t minimal_state(const minimal_t* instance, uapp_sm_state_t* state) {
    return instance != NULL ? uapp_sm_get_state(&instance->machine, state) : UETD_INVALID_ARG;
}
