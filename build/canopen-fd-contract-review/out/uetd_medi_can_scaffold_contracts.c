#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uproto/future/medi_can/medi_can.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_submit(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_submit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_submit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_process_frame(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_process_frame: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_process_frame: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_next_transmit(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_next_transmit: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_next_transmit: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_poll(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_execute_ready(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_execute_ready: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_execute_ready: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_record_result(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_record_result: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_record_result: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_next_event(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_next_event: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_next_event: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_snapshot(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_snapshot: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_snapshot: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_medi_can_reset(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_medi_can_reset: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_medi_can_reset: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_medi_can_scaffold_contracts check(s) failed across 11 operations\n", failures);
        return 1;
    }
    printf("Validated 11 uetd_medi_can_scaffold_contracts operations from 1 source files\n");
    return 0;
}
