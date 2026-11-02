#include "tasks.h"
#include "callbacks.h"

static uos_task_t owner_task = UOS_OBJECT_INITIALIZER;
/* Round the requested byte minimum up to a complete native stack element. */
static UETD_ALIGNAS(UOS_STORAGE_ALIGN) uos_stack_word_t owner_stack[(1024u + sizeof(uos_stack_word_t) - 1u) / sizeof(uos_stack_word_t)];
static void owner_entry(void* context) {
    uos_time_t next;
    uos_status_t os_status = uos_time_now(&next);
    if (os_status != UOS_OK) { manual_runtime_os_failure(os_status); }
    if (UINT64_C(1000) > UINT64_MAX - next) { manual_runtime_os_failure(UOS_OVERFLOW); }
    next += UINT64_C(1000);
    /* First activation is immediate; later releases retain the original cadence. */
    for (;;) {
        const uetd_status_t status = owner_step(context);
        if (status != UETD_OK) { manual_runtime_component_failure(status); }
        os_status = uos_task_delay_until(&next, UINT64_C(1000));
        if (os_status != UOS_OK) { manual_runtime_os_failure(os_status); }
    }
}

uos_status_t manual_tasks_create(void* const contexts[MANUAL_RUNTIME_TASK_COUNT]) {
    static bool attempted;
    static uos_status_t result = UOS_NOT_READY;
    if (attempted) { return result; }
    attempted = true;
    result = uos_task_create_static(&owner_task, "owner", owner_entry,
                                    contexts != NULL ? contexts[0] : NULL,
                                    2u, owner_stack, sizeof(owner_stack));
    if (result != UOS_OK) { return result; }
    return result;
}
