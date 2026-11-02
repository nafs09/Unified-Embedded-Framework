#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uos/uos.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_time_now(((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_time_now: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_time_now: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_duration_from_ms((uetd_u64_t){0}, ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_duration_from_ms: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_duration_from_ms: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_duration_from_s((uetd_u64_t){0}, ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_duration_from_s: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_duration_from_s: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_task_create_static(((void*)workspace.bytes), ((void*)workspace.bytes), (uos_task_fn_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_task_create_static: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_task_create_static: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_task_delay((uos_dur_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_task_delay: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_task_delay: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_task_delay_until(((void*)workspace.bytes), (uos_dur_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_task_delay_until: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_task_delay_until: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_task_yield() != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_task_yield: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_task_yield: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_scheduler_start() != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_scheduler_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_scheduler_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_mutex_create_static(((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_mutex_create_static: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_mutex_create_static: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_mutex_lock(((void*)workspace.bytes), (uos_dur_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_mutex_lock: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_mutex_lock: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_mutex_unlock(((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_mutex_unlock: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_mutex_unlock: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_sem_create_static(((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_sem_create_static: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_sem_create_static: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_sem_take(((void*)workspace.bytes), (uos_dur_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_sem_take: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_sem_take: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_sem_give(((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_sem_give: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_sem_give: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_sem_give_from_isr(((void*)workspace.bytes), ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_sem_give_from_isr: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_sem_give_from_isr: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_queue_create_static(((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u32_t){0}, ((void*)workspace.bytes), (size_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_queue_create_static: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_queue_create_static: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_queue_send(((void*)workspace.bytes), ((void*)workspace.bytes), (uos_dur_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_queue_send: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_queue_send: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_queue_recv(((void*)workspace.bytes), ((void*)workspace.bytes), (uos_dur_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_queue_recv: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_queue_recv: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_queue_send_from_isr(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_queue_send_from_isr: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_queue_send_from_isr: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_queue_waiting(((void*)workspace.bytes), ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_queue_waiting: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_queue_waiting: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_event_create_static(((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_event_create_static: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_event_create_static: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_event_valid_mask() != 0) {
        printf("FAIL uos_event_valid_mask: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_event_valid_mask: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_event_wait(((void*)workspace.bytes), (uetd_u32_t){0}, (uos_dur_t){0}, ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_event_wait: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_event_wait: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_event_set(((void*)workspace.bytes), (uetd_u32_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_event_set: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_event_set: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_event_clear(((void*)workspace.bytes), (uetd_u32_t){0}) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_event_clear: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_event_clear: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uos_event_set_from_isr(((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes)) != UOS_NOT_SUPPORTED) {
        printf("FAIL uos_event_set_from_isr: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uos_event_set_from_isr: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_uos_safertos_scaffold_contracts check(s) failed across 26 operations\n", failures);
        return 1;
    }
    printf("Validated 26 uetd_uos_safertos_scaffold_contracts operations from 1 source files\n");
    return 0;
}
