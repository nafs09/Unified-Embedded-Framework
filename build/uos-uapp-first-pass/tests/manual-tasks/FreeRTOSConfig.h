/* Portable static-kernel configuration for the pinned UETD FreeRTOS package. */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#include <stdint.h>
/* Supply this application-owned header on the include path. It owns the exact
 * port clock/IRQ settings, configASSERT, and the required UOS ISR-yield hook.
 * No generic MCU or board values are guessed here. */
#include "uos_port_config.h"
#ifndef configASSERT
#error "uos_port_config.h must provide a non-returning configASSERT failure path"
#endif
#ifndef UOS_FREERTOS_YIELD_FROM_ISR
#error "uos_port_config.h must provide a callable UOS_FREERTOS_YIELD_FROM_ISR(woken) hook"
#endif
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configTICK_RATE_HZ 1000UL
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS
#define configMAX_PRIORITIES 3U
#define configMAX_TASK_NAME_LEN 24U
#define configMINIMAL_STACK_SIZE 256U
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 0
#define configKERNEL_PROVIDED_STATIC_MEMORY 1
#define configUSE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 1U
#define configTIMER_TASK_STACK_DEPTH 512U
#define configTIMER_QUEUE_LENGTH 8U
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xSemaphoreGetMutexHolder 1
#define INCLUDE_xTimerPendFunctionCall 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskSuspend 1
#endif
