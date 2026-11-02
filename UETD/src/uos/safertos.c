/// @file src/uos/safertos.c
/// @brief Unselected licensed SafeRTOS port; no implementation is claimed.
#include "uos/uos.h"

uos_status_t uos_clock_bind(const uos_clock_provider_t* provider) {
    (void)provider;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_clock_use_platform(void) {
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_time_now(uos_time_t* now_us) {
    (void)now_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_duration_from_ms(uetd_u64_t ms, uos_dur_t* duration) {
    (void)ms;
    (void)duration;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_duration_from_s(uetd_u64_t seconds, uos_dur_t* duration) {
    (void)seconds;
    (void)duration;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_task_create_static(uos_task_t* task, const char* name, uos_task_fn_t fn, void* ctx,
                                    uetd_u32_t priority, void* stack, uetd_u32_t stack_bytes) {
    (void)task;
    (void)name;
    (void)fn;
    (void)ctx;
    (void)priority;
    (void)stack;
    (void)stack_bytes;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_task_delay(uos_dur_t duration_us) {
    (void)duration_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_task_delay_until(uos_time_t* next_us, uos_dur_t period_us) {
    (void)next_us;
    (void)period_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_task_yield(void) {
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_scheduler_start(void) {
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_mutex_create_static(uos_mutex_t* mutex) {
    (void)mutex;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_mutex_lock(uos_mutex_t* mutex, uos_dur_t timeout_us) {
    (void)mutex;
    (void)timeout_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_mutex_unlock(uos_mutex_t* mutex) {
    (void)mutex;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_sem_create_static(uos_sem_t* sem, uetd_u32_t initial, uetd_u32_t maximum) {
    (void)sem;
    (void)initial;
    (void)maximum;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_sem_take(uos_sem_t* sem, uos_dur_t timeout_us) {
    (void)sem;
    (void)timeout_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_sem_give(uos_sem_t* sem) {
    (void)sem;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_sem_give_from_isr(uos_sem_t* sem, bool* higher_priority_woken) {
    (void)sem;
    (void)higher_priority_woken;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_queue_create_static(uos_queue_t* queue, uetd_u32_t item_size, uetd_u32_t depth,
                                     void* item_buffer, size_t item_buffer_bytes) {
    (void)queue;
    (void)item_size;
    (void)depth;
    (void)item_buffer;
    (void)item_buffer_bytes;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_queue_send(uos_queue_t* queue, const void* item, uos_dur_t timeout_us) {
    (void)queue;
    (void)item;
    (void)timeout_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_queue_recv(uos_queue_t* queue, void* item, uos_dur_t timeout_us) {
    (void)queue;
    (void)item;
    (void)timeout_us;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_queue_send_from_isr(uos_queue_t* queue, const void* item, bool* higher_priority_woken) {
    (void)queue;
    (void)item;
    (void)higher_priority_woken;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_queue_waiting(const uos_queue_t* queue, uetd_u32_t* count) {
    (void)queue;
    (void)count;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_event_create_static(uos_event_t* event) {
    (void)event;
    return UOS_NOT_SUPPORTED;
}

uetd_u32_t uos_event_valid_mask(void) {
    return 0u;
}

uos_status_t uos_event_wait(uos_event_t* event, uetd_u32_t mask, uos_dur_t timeout_us, uetd_u32_t* matched) {
    (void)event;
    (void)mask;
    (void)timeout_us;
    (void)matched;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_event_set(uos_event_t* event, uetd_u32_t flags) {
    (void)event;
    (void)flags;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_event_clear(uos_event_t* event, uetd_u32_t flags) {
    (void)event;
    (void)flags;
    return UOS_NOT_SUPPORTED;
}

uos_status_t uos_event_set_from_isr(uos_event_t* event, uetd_u32_t flags, bool* higher_priority_woken) {
    (void)event;
    (void)flags;
    (void)higher_priority_woken;
    return UOS_NOT_SUPPORTED;
}
