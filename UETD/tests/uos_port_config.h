/* Test-only Windows port settings for the generated FreeRTOSConfig.h.
 * The task-template test replaces scheduling calls and never dispatches ISRs. */
#ifndef UOS_TEST_PORT_CONFIG_H
#define UOS_TEST_PORT_CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#define configCPU_CLOCK_HZ 100000000UL
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configASSERT(x) do { if (!(x)) { fprintf(stderr, "test config assertion failed: %s\n", #x); exit(2); } } while (0)
void uos_manual_test_isr_yield(int requested);
#define UOS_FREERTOS_YIELD_FROM_ISR(woken) uos_manual_test_isr_yield((woken) != 0)
#endif
