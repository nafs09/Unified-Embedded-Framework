/// @file src/uos/baremetal.c
/// @brief Single-loop backend: time/delay supported, kernel services explicit unsupported.
#include "uos/uos.h"
#include "detail.h"

uos_status_t uos_backend_task_ready(void) { return UOS_OK; }

uos_status_t uos_task_delay(uos_dur_t duration) {
    if (duration == UOS_WAIT_FOREVER) { return UOS_INVALID_PARAM; }
    uos_time_t start;
    uos_status_t status = uos_time_now(&start);
    if (status != UOS_OK) { return status; }
    if (duration > UINT64_MAX - start) { return UOS_OVERFLOW; }
    /* Chunk the low-level delay so its cycle conversion cannot saturate. */
    uos_time_t now = start;
    while (now - start < duration) {
        const uos_time_t before = now;
        const uos_dur_t remaining = duration - (now - start);
        status = uos_clock_delay(remaining > 1000000u ? 1000000u : remaining);
        if (status != UOS_OK) { return status; }
        status = uos_time_now(&now);
        if (status != UOS_OK) { return status; }
        if (now <= before) { return UOS_BACKEND_ERROR; }
    }
    return UOS_OK;
}
uos_status_t uos_task_yield(void) { return UOS_OK; }
uos_status_t uos_scheduler_start(void) { return UOS_NOT_SUPPORTED; }
uos_status_t uos_task_create_static(uos_task_t* task, const char* name, uos_task_fn_t fn, void* ctx,
                                    uetd_u32_t priority, void* stack, uetd_u32_t bytes) {
    (void)task; (void)name; (void)fn; (void)ctx; (void)priority; (void)stack; (void)bytes;
    return UOS_NOT_SUPPORTED;
}
uos_status_t uos_mutex_create_static(uos_mutex_t* m) { (void)m; return UOS_NOT_SUPPORTED; }
uos_status_t uos_mutex_lock(uos_mutex_t* m, uos_dur_t timeout) { (void)m; (void)timeout; return UOS_NOT_SUPPORTED; }
uos_status_t uos_mutex_unlock(uos_mutex_t* m) { (void)m; return UOS_NOT_SUPPORTED; }
uos_status_t uos_sem_create_static(uos_sem_t* s, uetd_u32_t initial, uetd_u32_t maximum) {
    (void)s; (void)initial; (void)maximum; return UOS_NOT_SUPPORTED;
}
uos_status_t uos_sem_take(uos_sem_t* s, uos_dur_t timeout) { (void)s; (void)timeout; return UOS_NOT_SUPPORTED; }
uos_status_t uos_sem_give(uos_sem_t* s) { (void)s; return UOS_NOT_SUPPORTED; }
uos_status_t uos_sem_give_from_isr(uos_sem_t* s, bool* woken) {
    (void)s; if (woken != NULL) { *woken = false; } return UOS_NOT_SUPPORTED;
}
uos_status_t uos_queue_create_static(uos_queue_t* q, uetd_u32_t size, uetd_u32_t depth, void* buffer, size_t bytes) {
    (void)q; (void)size; (void)depth; (void)buffer; (void)bytes; return UOS_NOT_SUPPORTED;
}
uos_status_t uos_queue_send(uos_queue_t* q, const void* item, uos_dur_t timeout) {
    (void)q; (void)item; (void)timeout; return UOS_NOT_SUPPORTED;
}
uos_status_t uos_queue_recv(uos_queue_t* q, void* item, uos_dur_t timeout) {
    (void)q; (void)item; (void)timeout; return UOS_NOT_SUPPORTED;
}
uos_status_t uos_queue_send_from_isr(uos_queue_t* q, const void* item, bool* woken) {
    (void)q; (void)item; if (woken != NULL) { *woken = false; } return UOS_NOT_SUPPORTED;
}
uos_status_t uos_queue_waiting(const uos_queue_t* q, uetd_u32_t* count) { (void)q; (void)count; return UOS_NOT_SUPPORTED; }
uos_status_t uos_event_create_static(uos_event_t* e) { (void)e; return UOS_NOT_SUPPORTED; }
uetd_u32_t uos_event_valid_mask(void) { return 0u; }
uos_status_t uos_event_wait(uos_event_t* e, uetd_u32_t mask, uos_dur_t timeout, uetd_u32_t* matched) {
    (void)e; (void)mask; (void)timeout; (void)matched; return UOS_NOT_SUPPORTED;
}
uos_status_t uos_event_set(uos_event_t* e, uetd_u32_t flags) { (void)e; (void)flags; return UOS_NOT_SUPPORTED; }
uos_status_t uos_event_clear(uos_event_t* e, uetd_u32_t flags) { (void)e; (void)flags; return UOS_NOT_SUPPORTED; }
uos_status_t uos_event_set_from_isr(uos_event_t* e, uetd_u32_t flags, bool* woken) {
    (void)e; (void)flags; if (woken != NULL) { *woken = false; } return UOS_NOT_SUPPORTED;
}
