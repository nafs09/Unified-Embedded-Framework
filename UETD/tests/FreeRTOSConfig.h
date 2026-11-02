#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCPU_CLOCK_HZ 100000000UL
#define configTICK_RATE_HZ 1000UL
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 256
#define configMAX_TASK_NAME_LEN 24
#ifndef UOS_TEST_TICK_WIDTH
#define UOS_TEST_TICK_WIDTH TICK_TYPE_WIDTH_32_BITS
#endif
#define configTICK_TYPE_WIDTH_IN_BITS UOS_TEST_TICK_WIDTH
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 0
#define configKERNEL_PROVIDED_STATIC_MEMORY 1
#define configUSE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 3
#define configTIMER_QUEUE_LENGTH 2
#define configTIMER_TASK_STACK_DEPTH 256
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xSemaphoreGetMutexHolder 1
#define INCLUDE_xTimerPendFunctionCall 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelete 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configASSERT(x) do { if (!(x)) { fprintf(stderr, "FreeRTOS assert %s:%d: %s\n", __FILE__, __LINE__, #x); fflush(stderr); exit(2); } } while (0)
/* The simulator dispatches a context switch from the handler return value.
 * Accumulate requests here; the outer native ISR returns the accumulated bit. */
void uos_test_request_isr_yield(int requested);
#define UOS_FREERTOS_YIELD_FROM_ISR(woken) uos_test_request_isr_yield((woken) != 0)
#endif
