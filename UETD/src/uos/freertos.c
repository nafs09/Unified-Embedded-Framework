/// @file src/uos/freertos.c
/// @brief Static FreeRTOS adapter; no finite wait maps to the forever sentinel.
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "ucore/assert.h"
#include "uos/uos.h"
#include "detail.h"
#include <stdint.h>
#ifndef UOS_FREERTOS_NATIVE_STORAGE
#error "Compile the adapter and every UOS consumer with UOS_FREERTOS_NATIVE_STORAGE=1"
#endif

/* Ports whose native macro returns from an ISR (e.g. the Windows simulator)
 * provide a callable yield hook here. It must return normally to this wrapper. */
#ifndef UOS_FREERTOS_YIELD_FROM_ISR
#error "Define UOS_FREERTOS_YIELD_FROM_ISR(woken) as a port hook that returns normally to the UOS wrapper"
#endif

#if configSUPPORT_STATIC_ALLOCATION != 1 || configUSE_MUTEXES != 1 || configUSE_COUNTING_SEMAPHORES != 1
#error "UOS requires static allocation, mutexes and counting semaphores"
#endif
#if INCLUDE_xTaskGetSchedulerState != 1 || INCLUDE_xTaskGetCurrentTaskHandle != 1 || INCLUDE_xSemaphoreGetMutexHolder != 1 || INCLUDE_vTaskDelay != 1
#error "UOS requires scheduler-state, mutex-holder and delay APIs"
#endif
#if configUSE_TIMERS != 1 || INCLUDE_xTimerPendFunctionCall != 1
#error "UOS ISR event flags require the timer daemon and pended function calls"
#endif
UETD_STATIC_ASSERT(configTICK_RATE_HZ > 0 && configTICK_RATE_HZ <= 1000000u, "UOS tick rate must be 1..1000000 Hz");
#define CHECK_STORAGE(native, object, bytes) \
    UETD_STATIC_ASSERT(sizeof(native) <= bytes, "Increase UOS control storage"); \
    UETD_STATIC_ASSERT(_Alignof(native) <= _Alignof(object), "Increase UOS storage alignment")
CHECK_STORAGE(StaticTask_t, uos_task_t, UOS_TASK_STORAGE_BYTES);
CHECK_STORAGE(StaticSemaphore_t, uos_mutex_t, UOS_MUTEX_STORAGE_BYTES);
CHECK_STORAGE(StaticSemaphore_t, uos_sem_t, UOS_SEM_STORAGE_BYTES);
CHECK_STORAGE(StaticQueue_t, uos_queue_t, UOS_QUEUE_STORAGE_BYTES);
CHECK_STORAGE(StaticEventGroup_t, uos_event_t, UOS_EVENT_STORAGE_BYTES);
#undef CHECK_STORAGE

static bool running(void) { return xTaskGetSchedulerState() == taskSCHEDULER_RUNNING; }
uos_status_t uos_backend_task_ready(void) { return running() ? UOS_OK : UOS_NOT_READY; }
static uos_status_t timeout_ticks(uos_dur_t us, TickType_t* result) {
    if (us == 0u) { *result = 0; return UOS_OK; }
    if (us == UOS_WAIT_FOREVER) {
#if INCLUDE_vTaskSuspend == 1
        *result = portMAX_DELAY; return UOS_OK;
#else
        return UOS_NOT_SUPPORTED;
#endif
    }
    const uint64_t rate = (uint64_t)configTICK_RATE_HZ;
    const uint64_t limit = (uint64_t)portMAX_DELAY - 1u;
    const uint64_t seconds = us / 1000000u;
    if (seconds > limit / rate) { return UOS_OVERFLOW; }
    const uint64_t whole = seconds * rate;
    const uint64_t part = ((us % 1000000u) * rate + 999999u) / 1000000u;
    /* One phase-guard tick prevents a near-tick-boundary call returning early. */
    if (part >= limit - whole) { return UOS_OVERFLOW; }
    *result = (TickType_t)(whole + part + 1u);
    return UOS_OK;
}
#define OBJECT_READY(object) do { \
    if ((object) == NULL) { return UOS_INVALID_PARAM; } \
    if ((object)->handle == NULL || !running()) { return UOS_NOT_READY; } \
} while (0)
#define WAIT_TICKS(timeout) TickType_t ticks; \
    const uos_status_t conversion = timeout_ticks(timeout, &ticks); \
    if (conversion != UOS_OK) { return conversion; }

uos_status_t uos_task_create_static(uos_task_t* task, const char* name, uos_task_fn_t fn, void* ctx,
                                    uetd_u32_t priority, void* stack, uetd_u32_t bytes) {
    if (task == NULL || name == NULL || fn == NULL || stack == NULL || bytes < sizeof(StackType_t) ||
        bytes % sizeof(StackType_t) != 0u || priority >= (uint64_t)configMAX_PRIORITIES ||
        ((uintptr_t)stack % portBYTE_ALIGNMENT) != 0u) { return UOS_INVALID_PARAM; }
    if (task->handle != NULL) { return UOS_BUSY; }
    const uint64_t depth = bytes / sizeof(StackType_t);
    if ((uint64_t)(configSTACK_DEPTH_TYPE)depth != depth || (uint64_t)(UBaseType_t)priority != priority) {
        return UOS_OVERFLOW;
    }
    task->handle = xTaskCreateStatic(fn, name, (configSTACK_DEPTH_TYPE)depth, ctx,
                                     (UBaseType_t)priority, (StackType_t*)stack, &task->control.native);
    return task->handle != NULL ? UOS_OK : UOS_BACKEND_ERROR;
}
uos_status_t uos_task_delay(uos_dur_t duration) {
    if (duration == UOS_WAIT_FOREVER) { return UOS_INVALID_PARAM; }
    if (!running()) { return UOS_NOT_READY; }
    WAIT_TICKS(duration);
    if (ticks == 0) { taskYIELD(); } else { vTaskDelay(ticks); }
    return UOS_OK;
}
uos_status_t uos_task_yield(void) {
    if (!running()) { return UOS_NOT_READY; }
    taskYIELD(); return UOS_OK;
}
uos_status_t uos_scheduler_start(void) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) { return UOS_BUSY; }
    vTaskStartScheduler();
    return UOS_BACKEND_ERROR; /* a successful embedded scheduler does not return */
}
uos_status_t uos_mutex_create_static(uos_mutex_t* m) {
    if (m == NULL) { return UOS_INVALID_PARAM; }
    if (m->handle != NULL) { return UOS_BUSY; }
    m->handle = xSemaphoreCreateMutexStatic(&m->control.native);
    return m->handle != NULL ? UOS_OK : UOS_BACKEND_ERROR;
}
uos_status_t uos_mutex_lock(uos_mutex_t* m, uos_dur_t timeout) {
    OBJECT_READY(m);
    if (xSemaphoreGetMutexHolder((SemaphoreHandle_t)m->handle) == xTaskGetCurrentTaskHandle()) { return UOS_BUSY; }
    WAIT_TICKS(timeout);
    return xSemaphoreTake((SemaphoreHandle_t)m->handle, ticks) == pdTRUE ? UOS_OK : timeout == 0 ? UOS_BUSY : UOS_TIMEOUT;
}
uos_status_t uos_mutex_unlock(uos_mutex_t* m) {
    OBJECT_READY(m);
    if (xSemaphoreGetMutexHolder((SemaphoreHandle_t)m->handle) != xTaskGetCurrentTaskHandle()) { return UOS_NOT_OWNER; }
    return xSemaphoreGive((SemaphoreHandle_t)m->handle) == pdTRUE ? UOS_OK : UOS_BACKEND_ERROR;
}
uos_status_t uos_sem_create_static(uos_sem_t* s, uetd_u32_t initial, uetd_u32_t maximum) {
    if (s == NULL || maximum == 0u || initial > maximum) { return UOS_INVALID_PARAM; }
    if (s->handle != NULL) { return UOS_BUSY; }
    if ((uint64_t)(UBaseType_t)maximum != maximum) { return UOS_OVERFLOW; }
    s->handle = xSemaphoreCreateCountingStatic((UBaseType_t)maximum, (UBaseType_t)initial, &s->control.native);
    return s->handle != NULL ? UOS_OK : UOS_BACKEND_ERROR;
}
uos_status_t uos_sem_take(uos_sem_t* s, uos_dur_t timeout) {
    OBJECT_READY(s);
    WAIT_TICKS(timeout);
    return xSemaphoreTake((SemaphoreHandle_t)s->handle, ticks) == pdTRUE ? UOS_OK : timeout == 0 ? UOS_EMPTY : UOS_TIMEOUT;
}
uos_status_t uos_sem_give(uos_sem_t* s) {
    OBJECT_READY(s);
    return xSemaphoreGive((SemaphoreHandle_t)s->handle) == pdTRUE ? UOS_OK : UOS_FULL;
}
uos_status_t uos_sem_give_from_isr(uos_sem_t* s, bool* woken) {
    if (woken != NULL) { *woken = false; }
    if (s == NULL) { return UOS_INVALID_PARAM; }
    if (s->handle == NULL) { return UOS_NOT_READY; }
    BaseType_t native_woken = pdFALSE;
    const BaseType_t result = xSemaphoreGiveFromISR((SemaphoreHandle_t)s->handle, &native_woken);
    if (woken != NULL) { *woken = native_woken != pdFALSE; }
    UOS_FREERTOS_YIELD_FROM_ISR(native_woken);
    return result == pdTRUE ? UOS_OK : UOS_FULL;
}
uos_status_t uos_queue_create_static(uos_queue_t* q, uetd_u32_t size, uetd_u32_t depth, void* buffer, size_t bytes) {
    if (q == NULL || buffer == NULL || size == 0u || depth == 0u) { return UOS_INVALID_PARAM; }
    if (q->handle != NULL) { return UOS_BUSY; }
    if ((uint64_t)(UBaseType_t)size != size || (uint64_t)(UBaseType_t)depth != depth ||
        (uint64_t)size * depth > SIZE_MAX) { return UOS_OVERFLOW; }
    if ((size_t)size * depth > bytes) { return UOS_INVALID_PARAM; }
    q->handle = xQueueCreateStatic((UBaseType_t)depth, (UBaseType_t)size, (uint8_t*)buffer, &q->control.native);
    return q->handle != NULL ? UOS_OK : UOS_BACKEND_ERROR;
}
uos_status_t uos_queue_send(uos_queue_t* q, const void* item, uos_dur_t timeout) {
    if (item == NULL) { return UOS_INVALID_PARAM; }
    OBJECT_READY(q);
    WAIT_TICKS(timeout);
    return xQueueSend((QueueHandle_t)q->handle, item, ticks) == pdTRUE ? UOS_OK : timeout == 0 ? UOS_FULL : UOS_TIMEOUT;
}
uos_status_t uos_queue_recv(uos_queue_t* q, void* item, uos_dur_t timeout) {
    if (item == NULL) { return UOS_INVALID_PARAM; }
    OBJECT_READY(q);
    WAIT_TICKS(timeout);
    return xQueueReceive((QueueHandle_t)q->handle, item, ticks) == pdTRUE ? UOS_OK : timeout == 0 ? UOS_EMPTY : UOS_TIMEOUT;
}
uos_status_t uos_queue_send_from_isr(uos_queue_t* q, const void* item, bool* woken) {
    if (woken != NULL) { *woken = false; }
    if (q == NULL || item == NULL) { return UOS_INVALID_PARAM; }
    if (q->handle == NULL) { return UOS_NOT_READY; }
    BaseType_t native_woken = pdFALSE;
    const BaseType_t result = xQueueSendFromISR((QueueHandle_t)q->handle, item, &native_woken);
    if (woken != NULL) { *woken = native_woken != pdFALSE; }
    UOS_FREERTOS_YIELD_FROM_ISR(native_woken);
    return result == pdTRUE ? UOS_OK : UOS_FULL;
}
uos_status_t uos_queue_waiting(const uos_queue_t* q, uetd_u32_t* count) {
    if (count == NULL) { return UOS_INVALID_PARAM; }
    OBJECT_READY(q);
    *count = (uetd_u32_t)uxQueueMessagesWaiting((QueueHandle_t)q->handle);
    return UOS_OK;
}
uos_status_t uos_event_create_static(uos_event_t* e) {
    if (e == NULL) { return UOS_INVALID_PARAM; }
    if (e->handle != NULL) { return UOS_BUSY; }
    e->handle = xEventGroupCreateStatic(&e->control.native);
    return e->handle != NULL ? UOS_OK : UOS_BACKEND_ERROR;
}
uetd_u32_t uos_event_valid_mask(void) { return (uetd_u32_t)(~(EventBits_t)eventEVENT_BITS_CONTROL_BYTES); }
static bool valid_flags(uetd_u32_t flags) { return flags != 0u && (flags & ~uos_event_valid_mask()) == 0u; }
uos_status_t uos_event_wait(uos_event_t* e, uetd_u32_t mask, uos_dur_t timeout, uetd_u32_t* matched) {
    if (matched == NULL || !valid_flags(mask)) { return UOS_INVALID_PARAM; }
    OBJECT_READY(e);
    WAIT_TICKS(timeout);
    const uetd_u32_t bits = (uetd_u32_t)xEventGroupWaitBits((EventGroupHandle_t)e->handle, (EventBits_t)mask,
                                                         pdTRUE, pdFALSE, ticks) & mask;
    if (bits == 0u) { return timeout == 0u ? UOS_EMPTY : UOS_TIMEOUT; }
    *matched = bits;
    return UOS_OK;
}
uos_status_t uos_event_set(uos_event_t* e, uetd_u32_t flags) {
    if (!valid_flags(flags)) { return UOS_INVALID_PARAM; }
    OBJECT_READY(e);
    (void)xEventGroupSetBits((EventGroupHandle_t)e->handle, (EventBits_t)flags);
    return UOS_OK;
}
uos_status_t uos_event_clear(uos_event_t* e, uetd_u32_t flags) {
    if (!valid_flags(flags)) { return UOS_INVALID_PARAM; }
    OBJECT_READY(e);
    (void)xEventGroupClearBits((EventGroupHandle_t)e->handle, (EventBits_t)flags);
    return UOS_OK;
}
uos_status_t uos_event_set_from_isr(uos_event_t* e, uetd_u32_t flags, bool* woken) {
    if (woken != NULL) { *woken = false; }
    if (e == NULL || !valid_flags(flags)) { return UOS_INVALID_PARAM; }
    if (e->handle == NULL) { return UOS_NOT_READY; }
    BaseType_t native_woken = pdFALSE;
    const BaseType_t result = xEventGroupSetBitsFromISR((EventGroupHandle_t)e->handle, (EventBits_t)flags, &native_woken);
    if (woken != NULL) { *woken = native_woken != pdFALSE; }
    UOS_FREERTOS_YIELD_FROM_ISR(native_woken);
    return result == pdPASS ? UOS_OK : UOS_FULL;
}
