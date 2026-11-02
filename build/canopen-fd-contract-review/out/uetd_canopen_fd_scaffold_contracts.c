#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uproto/future/industrial/canopen_fd.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_process_frame(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_process_frame: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_process_frame: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_next_event(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_next_event: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_next_event: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_poll(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_usdo_read(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u16_t){0}, (uetd_u8_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_usdo_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_usdo_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_usdo_write(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u16_t){0}, (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_usdo_write: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_usdo_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_fd_reset(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_fd_reset: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_fd_reset: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_canopen_fd_scaffold_contracts check(s) failed across 8 operations\n", failures);
        return 1;
    }
    printf("Validated 8 uetd_canopen_fd_scaffold_contracts operations from 1 source files\n");
    return 0;
}
