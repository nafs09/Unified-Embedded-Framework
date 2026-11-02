#include "uos_uapp_test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uos/uos.h"
#include "ucore/time.h"
static uos_task_t task;
static UETD_ALIGNAS(UOS_STORAGE_ALIGN) StackType_t stack[512];
static uos_mutex_t mutex;
static uos_sem_t semaphore;
static uos_queue_t queue;
static uos_event_t event;
static unsigned queue_storage[2];
static volatile unsigned irq_done;
static unsigned yield_requested;
void uos_test_request_isr_yield(int requested) { if (requested) { yield_requested = 1; } }
static uint32_t simulated_irq(void) {
    yield_requested = 0;
    bool woken = true; unsigned item = 3;
    CHECK(uos_queue_send_from_isr(&queue, &item, &woken) == UOS_FULL && !woken);
    CHECK(uos_sem_give_from_isr(&semaphore, &woken) == UOS_FULL);
    CHECK(uos_event_set_from_isr(&event, 1, &woken) == UOS_OK);
    CHECK(uos_event_set_from_isr(&event, 2, &woken) == UOS_OK);
    CHECK(uos_event_set_from_isr(&event, 4, &woken) == UOS_FULL);
    irq_done = 1;
    return yield_requested;
}
static void run(void* context) {
    fprintf(stderr, "scheduler task started\n");
    (void)context; unsigned item = 7, out = 99, count = 99; uetd_u32_t flags = 99;
    CHECK(uos_mutex_unlock(&mutex) == UOS_NOT_OWNER);
    CHECK(uos_mutex_lock(&mutex, 0) == UOS_OK);
    CHECK(uos_mutex_lock(&mutex, 0) == UOS_BUSY);
    CHECK(uos_mutex_unlock(&mutex) == UOS_OK);
    CHECK(uos_sem_take(&semaphore, 0) == UOS_EMPTY);
    CHECK(uos_sem_take(&semaphore, 1) == UOS_TIMEOUT);
    CHECK(uos_sem_give(&semaphore) == UOS_OK);
    CHECK(uos_sem_give(&semaphore) == UOS_FULL);
    CHECK(uos_queue_recv(&queue, &out, 0) == UOS_EMPTY && out == 99);
    CHECK(uos_queue_send(&queue, &item, 0) == UOS_OK); item = 8;
    CHECK(uos_queue_send(&queue, &item, 0) == UOS_OK);
    CHECK(uos_queue_send(&queue, &item, 0) == UOS_FULL);
    CHECK(uos_queue_waiting(&queue, &count) == UOS_OK && count == 2);
    CHECK(uos_event_wait(&event, 1, 0, &flags) == UOS_EMPTY && flags == 99);
    CHECK(uos_event_set(&event, 3) == UOS_OK);
    CHECK(uos_event_wait(&event, 1, 0, &flags) == UOS_OK && flags == 1);
    CHECK(uos_event_wait(&event, 2, 0, &flags) == UOS_OK && flags == 2);
    CHECK(uos_event_set(&event, 0) == UOS_INVALID_PARAM);
    if (uos_event_valid_mask() != UINT32_MAX) {
        CHECK(uos_event_set(&event, ~uos_event_valid_mask()) == UOS_INVALID_PARAM);
    }
    if (sizeof(TickType_t) < 8) {
        CHECK(uos_queue_send(&queue, &item, UINT64_MAX - 1) == UOS_OVERFLOW);
        CHECK(uos_task_delay(UINT64_MAX - 1) == UOS_OVERFLOW);
    }
    CHECK(uos_task_delay(UOS_WAIT_FOREVER) == UOS_INVALID_PARAM);
    CHECK(uos_task_delay(1) == UOS_OK);
    fprintf(stderr, "task primitives passed; entering simulated IRQ\n");
    vPortSetInterruptHandler(portINTERRUPT_APPLICATION_DEFINED_START, simulated_irq);
    vPortGenerateSimulatedInterrupt(portINTERRUPT_APPLICATION_DEFINED_START);
    for (unsigned i = 0; !irq_done && i < 100; ++i) { CHECK(uos_task_delay(1000) == UOS_OK); }
    CHECK(irq_done);
    CHECK(uos_event_wait(&event, 3, UOS_MS(20), &flags) == UOS_OK && flags == 3);
    CHECK(uos_queue_recv(&queue, &out, 0) == UOS_OK && out == 7);
    CHECK(uos_queue_recv(&queue, &out, 0) == UOS_OK && out == 8);
    uos_time_t before, after;
    CHECK(uos_time_now(&before) == UOS_OK); CHECK(uos_task_delay(1000) == UOS_OK);
    CHECK(uos_time_now(&after) == UOS_OK && after >= before + 1000);
    CHECK(after <= uetd_time_now_us());
    puts("UOS against the real FreeRTOS kernel and simulated IRQs passed"); fflush(stdout); exit(0);
}
int main(void) {
    fprintf(stderr, "creating static kernel objects\n");
    CHECK(uos_clock_use_platform() == UOS_OK);
    CHECK(uos_task_delay(0) == UOS_NOT_READY);
    uos_time_t next = 0;
    CHECK(uos_task_delay_until(&next, 1) == UOS_NOT_READY && next == 0);
    CHECK(uos_mutex_create_static(&mutex) == UOS_OK);
    CHECK(uos_mutex_create_static(&mutex) == UOS_BUSY);
    CHECK(uos_sem_create_static(&semaphore, 0, 1) == UOS_OK);
    CHECK(uos_queue_create_static(&queue, sizeof(unsigned), 2, queue_storage, sizeof(queue_storage) - 1) == UOS_INVALID_PARAM);
    CHECK(uos_queue_create_static(&queue, sizeof(unsigned), 2, queue_storage, sizeof(queue_storage)) == UOS_OK);
    CHECK(uos_event_create_static(&event) == UOS_OK);
    CHECK(uos_task_create_static(&task, "contracts", run, NULL, 2, stack, sizeof(stack)) == UOS_OK);
    CHECK(uos_task_create_static(&task, "duplicate", run, NULL, 2, stack, sizeof(stack)) == UOS_BUSY);
    fprintf(stderr, "starting scheduler\n");
    (void)uos_scheduler_start(); CHECK(false); return 1;
}
