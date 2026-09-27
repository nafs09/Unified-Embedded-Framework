/// @file src/uos/freertos/uos_freertos.c
/// @brief Static-allocation FreeRTOS adapter for the public UOS contract.

#include "uef/uos/uos.h"
#include "uef/ucore/uef_assert.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <limits.h>
#include <stdint.h>

UEF_STATIC_ASSERT(sizeof(StaticTask_t) <= UOS_TASK_STORAGE_BYTES,
                  "UOS_TASK_STORAGE_BYTES is too small for this FreeRTOS port");
UEF_STATIC_ASSERT(sizeof(StaticSemaphore_t) <= UOS_MUTEX_STORAGE_BYTES,
                  "UOS_MUTEX_STORAGE_BYTES is too small for this FreeRTOS port");
UEF_STATIC_ASSERT(sizeof(StaticSemaphore_t) <= UOS_SEM_STORAGE_BYTES,
                  "UOS_SEM_STORAGE_BYTES is too small for this FreeRTOS port");
UEF_STATIC_ASSERT(sizeof(StaticQueue_t) <= UOS_QUEUE_STORAGE_BYTES,
                  "UOS_QUEUE_STORAGE_BYTES is too small for this FreeRTOS port");
UEF_STATIC_ASSERT(sizeof(StaticEventGroup_t) <= UOS_EVENT_STORAGE_BYTES,
                  "UOS_EVENT_STORAGE_BYTES is too small for this FreeRTOS port");
UEF_STATIC_ASSERT(_Alignof(StaticTask_t) <= UOS_STORAGE_ALIGN,
                  "Increase UOS_STORAGE_ALIGN for this FreeRTOS port");
UEF_STATIC_ASSERT(_Alignof(StaticSemaphore_t) <= UOS_STORAGE_ALIGN,
                  "Increase UOS_STORAGE_ALIGN for FreeRTOS semaphore storage");
UEF_STATIC_ASSERT(_Alignof(StaticQueue_t) <= UOS_STORAGE_ALIGN,
                  "Increase UOS_STORAGE_ALIGN for FreeRTOS queue storage");
UEF_STATIC_ASSERT(_Alignof(StaticEventGroup_t) <= UOS_STORAGE_ALIGN,
                  "Increase UOS_STORAGE_ALIGN for FreeRTOS event storage");

static TickType_t timeout_ticks(uos_dur_t timeout_us) {
    if (timeout_us == 0u) {
        return 0;
    }
    if (timeout_us == UOS_WAIT_FOREVER) {
        return portMAX_DELAY;
    }

    /* Round positive durations up: a nonzero UOS wait must not turn into an immediate poll. */
    const uint64_t rate = (uint64_t)configTICK_RATE_HZ;
    if (rate != 0u && timeout_us > (UINT64_MAX - 999999u) / rate) {
        return portMAX_DELAY;
    }
    uint64_t ticks = (timeout_us * rate + 999999u) / 1000000u;
    if (ticks == 0u) {
        ticks = 1u; /* A positive delay must not become a non-blocking poll. */
    }
    return ticks > (uint64_t)portMAX_DELAY ? portMAX_DELAY : (TickType_t)ticks;
}

static uos_dur_t ticks_to_us(TickType_t ticks) {
    const uint64_t rate = (uint64_t)configTICK_RATE_HZ;
    if (rate == 0u || (uint64_t)ticks > UINT64_MAX / 1000000u) {
        return 0u;
    }
    /* Tick-to-time conversion truncates sub-microsecond fractions, which cannot be represented. */
    return ((uint64_t)ticks * 1000000u) / rate;
}

uos_time_t uos_time_now(void) {
    return ticks_to_us(xTaskGetTickCount());
}

bool uos_task_create_static(uos_task_t* task, const char* name,
                            uos_task_fn_t function, void* context,
                            uef_u32_t priority, void* stack,
                            uef_u32_t stack_bytes) {
    if (task == NULL || name == NULL || function == NULL || stack == NULL ||
        stack_bytes < sizeof(StackType_t) ||
        stack_bytes % sizeof(StackType_t) != 0u ||
        priority >= (uef_u32_t)configMAX_PRIORITIES ||
        ((uintptr_t)stack % _Alignof(StackType_t)) != 0u) {
        return false;
    }

    /* FreeRTOS sizes stacks in StackType_t words; the public UOS contract accepts bytes. */
    const uint32_t stack_depth = (uint32_t)(stack_bytes / sizeof(StackType_t));
    return xTaskCreateStatic(function, name, stack_depth, context,
        (UBaseType_t)priority, (StackType_t*)stack,
        (StaticTask_t*)(void*)task->storage) != NULL;
}

void uos_task_delay(uos_dur_t duration_us) {
    const TickType_t ticks = timeout_ticks(duration_us);
    if (ticks == 0) {
        taskYIELD();
    } else {
        vTaskDelay(ticks);
    }
}

void uos_task_delay_until(uos_time_t* next_us, uos_dur_t period_us) {
    if (next_us == NULL || period_us == 0u) {
        return;
    }

    const uos_time_t now = uos_time_now();
    if (*next_us > now) {
        uos_task_delay(*next_us - now);
    }
    *next_us += period_us;
    const uos_time_t after_delay = uos_time_now();
    if (*next_us <= after_delay) {
        const uos_dur_t missed_periods = (after_delay - *next_us) / period_us + 1u;
        if (missed_periods <= (UINT64_MAX - *next_us) / period_us) {
            *next_us += missed_periods * period_us;
        }
    }
}

void uos_task_yield(void) {
    taskYIELD();
}

void uos_scheduler_start(void) {
    vTaskStartScheduler();
}

bool uos_mutex_create_static(uos_mutex_t* mutex) {
    return mutex != NULL && xSemaphoreCreateMutexStatic(
        (StaticSemaphore_t*)(void*)mutex->storage) != NULL;
}

bool uos_mutex_lock(uos_mutex_t* mutex, uos_dur_t timeout_us) {
    return mutex != NULL && xSemaphoreTake(
        (SemaphoreHandle_t)(void*)mutex->storage,
        timeout_ticks(timeout_us)) == pdTRUE;
}

void uos_mutex_unlock(uos_mutex_t* mutex) {
    if (mutex != NULL) {
        (void)xSemaphoreGive((SemaphoreHandle_t)(void*)mutex->storage);
    }
}

bool uos_sem_create_static(uos_sem_t* sem, uef_u32_t initial,
                           uef_u32_t maximum) {
    if (sem == NULL || maximum == 0u || initial > maximum ||
        maximum > (uef_u32_t)((UBaseType_t)~(UBaseType_t)0)) {
        return false;
    }
    return xSemaphoreCreateCountingStatic((UBaseType_t)maximum,
        (UBaseType_t)initial, (StaticSemaphore_t*)(void*)sem->storage) != NULL;
}

bool uos_sem_take(uos_sem_t* sem, uos_dur_t timeout_us) {
    return sem != NULL && xSemaphoreTake(
        (SemaphoreHandle_t)(void*)sem->storage,
        timeout_ticks(timeout_us)) == pdTRUE;
}

void uos_sem_give(uos_sem_t* sem) {
    if (sem != NULL) {
        (void)xSemaphoreGive((SemaphoreHandle_t)(void*)sem->storage);
    }
}

void uos_sem_give_from_isr(uos_sem_t* sem, bool* higher_priority_woken) {
    BaseType_t higher_priority_woken_native = pdFALSE;
    if (sem == NULL) {
        if (higher_priority_woken != NULL) *higher_priority_woken = false;
        return;
    }
    (void)xSemaphoreGiveFromISR((SemaphoreHandle_t)(void*)sem->storage,
                                &higher_priority_woken_native);
    if (higher_priority_woken != NULL) {
        *higher_priority_woken = higher_priority_woken_native == pdTRUE;
    }
    portYIELD_FROM_ISR(higher_priority_woken_native);
}

bool uos_queue_create_static(uos_queue_t* queue, uef_u32_t item_size,
                             uef_u32_t depth, void* item_buffer) {
    if (queue == NULL || item_buffer == NULL || item_size == 0u || depth == 0u ||
        (uint64_t)item_size * (uint64_t)depth > SIZE_MAX) {
        return false;
    }
    return xQueueCreateStatic((UBaseType_t)depth, (UBaseType_t)item_size,
        (uint8_t*)item_buffer,
        (StaticQueue_t*)(void*)queue->storage) != NULL;
}

bool uos_queue_send(uos_queue_t* queue, const void* item,
                    uos_dur_t timeout_us) {
    return queue != NULL && item != NULL && xQueueSend(
        (QueueHandle_t)(void*)queue->storage, item,
        timeout_ticks(timeout_us)) == pdTRUE;
}

bool uos_queue_recv(uos_queue_t* queue, void* item, uos_dur_t timeout_us) {
    return queue != NULL && item != NULL && xQueueReceive(
        (QueueHandle_t)(void*)queue->storage, item,
        timeout_ticks(timeout_us)) == pdTRUE;
}

bool uos_queue_send_from_isr(uos_queue_t* queue, const void* item,
                             bool* higher_priority_woken) {
    BaseType_t higher_priority_woken_native = pdFALSE;
    if (queue == NULL || item == NULL) {
        if (higher_priority_woken != NULL) *higher_priority_woken = false;
        return false;
    }
    const BaseType_t sent = xQueueSendFromISR(
        (QueueHandle_t)(void*)queue->storage, item,
        &higher_priority_woken_native);
    if (higher_priority_woken != NULL) {
        *higher_priority_woken = higher_priority_woken_native == pdTRUE;
    }
    portYIELD_FROM_ISR(higher_priority_woken_native);
    return sent == pdTRUE;
}

uef_u32_t uos_queue_waiting(const uos_queue_t* queue) {
    return queue != NULL
        ? (uef_u32_t)uxQueueMessagesWaiting((QueueHandle_t)(void*)queue->storage)
        : 0u;
}

bool uos_event_create_static(uos_event_t* event) {
    return event != NULL && xEventGroupCreateStatic(
        (StaticEventGroup_t*)(void*)event->storage) != NULL;
}

uef_u32_t uos_event_wait(uos_event_t* event, uef_u32_t mask,
                         uos_dur_t timeout_us) {
    if (event == NULL || mask == 0u) {
        return 0u;
    }
    return (uef_u32_t)xEventGroupWaitBits(
        (EventGroupHandle_t)(void*)event->storage, (EventBits_t)mask,
        pdTRUE, pdFALSE, timeout_ticks(timeout_us));
}

void uos_event_set(uos_event_t* event, uef_u32_t flags) {
    if (event != NULL) {
        (void)xEventGroupSetBits((EventGroupHandle_t)(void*)event->storage,
                                 (EventBits_t)flags);
    }
}

void uos_event_clear(uos_event_t* event, uef_u32_t flags) {
    if (event != NULL) {
        (void)xEventGroupClearBits((EventGroupHandle_t)(void*)event->storage,
                                   (EventBits_t)flags);
    }
}

void uos_event_set_from_isr(uos_event_t* event, uef_u32_t flags,
                            bool* higher_priority_woken) {
    BaseType_t higher_priority_woken_native = pdFALSE;
    if (event == NULL) {
        if (higher_priority_woken != NULL) *higher_priority_woken = false;
        return;
    }
    (void)xEventGroupSetBitsFromISR(
        (EventGroupHandle_t)(void*)event->storage, (EventBits_t)flags,
        &higher_priority_woken_native);
    if (higher_priority_woken != NULL) {
        *higher_priority_woken = higher_priority_woken_native == pdTRUE;
    }
    portYIELD_FROM_ISR(higher_priority_woken_native);
}
