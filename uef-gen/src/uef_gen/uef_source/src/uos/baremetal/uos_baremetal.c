/// @file src/uos/baremetal/uos_baremetal.c
/// @brief Minimal single-loop UOS backend for projects without an RTOS.

#include "uef/uos/uos.h"
#include "uef/ucore/uef_time.h"

#include <stdint.h>

uos_time_t uos_time_now(void) {
    return uef_time_now_us();
}

bool uos_task_create_static(uos_task_t* task, const char* name,
                            uos_task_fn_t function, void* context,
                            uef_u32_t priority, void* stack,
                            uef_u32_t stack_bytes) {
    /* This backend intentionally creates no task: application code owns the main loop. Returning
     * false prevents callers from mistaking a stored descriptor for a scheduled task. */
    (void)task;
    (void)name;
    (void)function;
    (void)context;
    (void)priority;
    (void)stack;
    (void)stack_bytes;
    return false;
}

void uos_task_delay(uos_dur_t duration_us) {
    uef_delay_us(duration_us);
}

void uos_task_delay_until(uos_time_t* next_us, uos_dur_t period_us) {
    if (next_us == NULL || period_us == 0u) {
        return;
    }

    const uos_time_t now = uos_time_now();
    if (*next_us > now) {
        uos_task_delay(*next_us - now);
    }

    const uos_time_t after_delay = uos_time_now();
    *next_us += period_us;
    if (*next_us <= after_delay) {
        const uos_dur_t missed_periods = (after_delay - *next_us) / period_us + 1u;
        if (missed_periods <= (UINT64_MAX - *next_us) / period_us) {
            *next_us += missed_periods * period_us;
        }
    }
}

void uos_task_yield(void) {
    /* There is no other runnable task in this single-loop backend. */
}

void uos_scheduler_start(void) {
    /* Application code owns the main loop in a bare-metal project. */
}

/* These facilities fail explicitly because a cooperative main loop cannot provide blocking
 * synchronization semantics. Use the FreeRTOS backend or implement a board-owned UOS backend. */
bool uos_mutex_create_static(uos_mutex_t* mutex) {
    (void)mutex;
    return false;
}

bool uos_mutex_lock(uos_mutex_t* mutex, uos_dur_t timeout_us) {
    (void)mutex;
    (void)timeout_us;
    return false;
}

void uos_mutex_unlock(uos_mutex_t* mutex) {
    (void)mutex;
}

bool uos_sem_create_static(uos_sem_t* sem, uef_u32_t initial,
                           uef_u32_t maximum) {
    (void)sem;
    (void)initial;
    (void)maximum;
    return false;
}

bool uos_sem_take(uos_sem_t* sem, uos_dur_t timeout_us) {
    (void)sem;
    (void)timeout_us;
    return false;
}

void uos_sem_give(uos_sem_t* sem) {
    (void)sem;
}

void uos_sem_give_from_isr(uos_sem_t* sem, bool* higher_prio_woken) {
    (void)sem;
    if (higher_prio_woken != NULL) {
        *higher_prio_woken = false;
    }
}

bool uos_queue_create_static(uos_queue_t* queue, uef_u32_t item_size,
                             uef_u32_t depth, void* item_buffer) {
    (void)queue;
    (void)item_size;
    (void)depth;
    (void)item_buffer;
    return false;
}

bool uos_queue_send(uos_queue_t* queue, const void* item,
                    uos_dur_t timeout_us) {
    (void)queue;
    (void)item;
    (void)timeout_us;
    return false;
}

bool uos_queue_recv(uos_queue_t* queue, void* item, uos_dur_t timeout_us) {
    (void)queue;
    (void)item;
    (void)timeout_us;
    return false;
}

bool uos_queue_send_from_isr(uos_queue_t* queue, const void* item,
                             bool* higher_prio_woken) {
    (void)queue;
    (void)item;
    if (higher_prio_woken != NULL) {
        *higher_prio_woken = false;
    }
    return false;
}

uef_u32_t uos_queue_waiting(const uos_queue_t* queue) {
    (void)queue;
    return 0u;
}

bool uos_event_create_static(uos_event_t* event) {
    (void)event;
    return false;
}

uef_u32_t uos_event_wait(uos_event_t* event, uef_u32_t mask,
                         uos_dur_t timeout_us) {
    (void)event;
    (void)mask;
    (void)timeout_us;
    return 0u;
}

void uos_event_set(uos_event_t* event, uef_u32_t flags) {
    (void)event;
    (void)flags;
}

void uos_event_clear(uos_event_t* event, uef_u32_t flags) {
    (void)event;
    (void)flags;
}

void uos_event_set_from_isr(uos_event_t* event, uef_u32_t flags,
                            bool* higher_prio_woken) {
    (void)event;
    (void)flags;
    if (higher_prio_woken != NULL) {
        *higher_prio_woken = false;
    }
}
