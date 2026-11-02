#include "uos_uapp_test.h"
#include "uos/uos.h"
#include "ucore/time.h"
static uetd_u64_t clock_us;
static bool clock_ready = true;
static unsigned waits;
uetd_time_us_t uetd_time_now_us(void) { return clock_us; }
void uetd_delay_us(uetd_dur_us_t duration) { clock_us += duration; ++waits; }
static uos_status_t read_clock(void* ctx, uos_time_t* value) {
    (void)ctx; *value = clock_us; return clock_ready ? UOS_OK : UOS_NOT_READY;
}
static uos_status_t delay_clock(void* ctx, uos_dur_t duration) { (void)ctx; uetd_delay_us(duration); return UOS_OK; }
int main(void) {
    uos_time_t output = 99;
    CHECK(uos_time_now(&output) == UOS_NOT_READY && output == 99);
    const uos_clock_provider_t provider = {read_clock, delay_clock, NULL};
    CHECK(uos_clock_bind(&provider) == UOS_OK);
    CHECK(uos_clock_use_platform() == UOS_BUSY);
    clock_ready = false; CHECK(uos_time_now(&output) == UOS_NOT_READY && output == 99);
    clock_ready = true; CHECK(uos_time_now(&output) == UOS_OK && output == 0);
    CHECK(uos_duration_from_ms(UINT64_MAX, &output) == UOS_OVERFLOW && output == 0);
    CHECK(uos_duration_from_s(UINT64_MAX, &output) == UOS_OVERFLOW && output == 0);
    CHECK(uos_duration_from_ms(2, &output) == UOS_OK && output == 2000);
    CHECK(uos_task_delay(UOS_WAIT_FOREVER) == UOS_INVALID_PARAM && waits == 0);
    output = 10; CHECK(uos_task_delay_until(&output, 5) == UOS_OK && output == 15 && clock_us == 10);
    clock_us = 30; CHECK(uos_task_delay_until(&output, 5) == UOS_OK && output == 35 && waits == 1);
    output = UINT64_MAX - 2;
    CHECK(uos_task_delay_until(&output, 3) == UOS_OVERFLOW && output == UINT64_MAX - 2 && waits == 1);
    output = 0; clock_us = UINT64_MAX;
    CHECK(uos_task_delay_until(&output, 1) == UOS_OVERFLOW && output == 0);
    CHECK(uos_task_delay(1) == UOS_OVERFLOW);
    uos_queue_t q = {0}; unsigned count = 99; bool woken = true;
    CHECK(uos_queue_waiting(&q, &count) == UOS_NOT_SUPPORTED && count == 99);
    CHECK(uos_queue_send_from_isr(&q, &count, &woken) == UOS_NOT_SUPPORTED && !woken);
    CHECK(uos_scheduler_start() == UOS_NOT_SUPPORTED);
    puts("UOS epoch, finite conversion, deadlines and bare-metal capabilities passed"); return 0;
}
