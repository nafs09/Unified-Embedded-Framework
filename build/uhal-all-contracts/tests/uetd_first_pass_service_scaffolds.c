#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <umid/data/storage.h>
#include <umid/data/calibration.h>
#include <umid/logging/blackbox.h>
#include <umid/data/process_image.h>
#include <uproto/industrial/modbus.h>
#include <uproto/can/canopen.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_store_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_store_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_store_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_store_read(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_store_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_store_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_store_write_atomic(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_store_write_atomic: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_store_write_atomic: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_store_erase(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_store_erase: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_store_erase: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_calibration_store_save(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_calibration_store_save: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_calibration_store_save: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_calibration_store_load(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u16_t){0}, ((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_calibration_store_load: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_calibration_store_load: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_blackbox_validate(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_blackbox_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_blackbox_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_blackbox_start(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_blackbox_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_blackbox_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_blackbox_append(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_blackbox_append: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_blackbox_append: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_blackbox_flush(((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_blackbox_flush: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_blackbox_flush: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_blackbox_get_stats(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_blackbox_get_stats: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_blackbox_get_stats: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_blackbox_stop(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_blackbox_stop: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_blackbox_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_process_image_init(((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_process_image_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_process_image_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_process_image_begin_scan(((void*)workspace.bytes), (uetd_u32_t){0}, (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_process_image_begin_scan: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_process_image_begin_scan: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_process_image_stage_point(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_process_image_stage_point: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_process_image_stage_point: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_process_image_commit_scan(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_process_image_commit_scan: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_process_image_commit_scan: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_process_image_snapshot(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_process_image_snapshot: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_process_image_snapshot: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_feed(((void*)workspace.bytes), ((void*)workspace.bytes), (uetd_u16_t){0}, (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_feed: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_feed: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_next_event(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_next_event: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_next_event: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_build_request(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u16_t){0}, (uetd_u16_t){0}, ((void*)workspace.bytes), (uetd_u16_t){0}, ((void*)workspace.bytes), (uetd_u16_t){0}, ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_build_request: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_build_request: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_poll(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_modbus_reset(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_modbus_reset: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_modbus_reset: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_validate(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_validate: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_validate: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_init(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_process_frame(((void*)workspace.bytes), (uetd_u32_t){0}, ((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_process_frame: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_process_frame: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_next_event(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_next_event: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_next_event: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_poll(((void*)workspace.bytes), (uetd_u64_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_poll: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_poll: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_sdo_read(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u16_t){0}, (uetd_u8_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_sdo_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_sdo_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_sdo_write(((void*)workspace.bytes), (uetd_u8_t){0}, (uetd_u16_t){0}, (uetd_u8_t){0}, ((void*)workspace.bytes), (uetd_u16_t){0}) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_sdo_write: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_sdo_write: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (uproto_canopen_reset(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL uproto_canopen_reset: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL uproto_canopen_reset: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_first_pass_service_scaffolds check(s) failed across 32 operations\n", failures);
        return 1;
    }
    printf("Validated 32 uetd_first_pass_service_scaffolds operations from 6 source files\n");
    return 0;
}
