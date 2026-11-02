#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <umid/drivers/current.h>
#include <umid/drivers/encoder.h>
#include <umid/drivers/gnss.h>
#include <umid/drivers/imu.h>
#include <umid/drivers/magnetometer.h>
#include <umid/drivers/pressure.h>
#include <umid/drivers/range.h>
#include <umid/drivers/temperature.h>
#include <umid/drivers/voltage.h>

static union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;
static unsigned char before[sizeof(workspace.bytes)];

int main(void) {
    size_t failures = 0u;
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_current_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_current_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_current_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_current_read_a(((void*)workspace.bytes)) != 0) {
        printf("FAIL umid_current_read_a: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_current_read_a: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_current_calibrate_zero(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_current_calibrate_zero: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_current_calibrate_zero: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_encoder_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_encoder_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_encoder_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_encoder_count(((void*)workspace.bytes)) != 0) {
        printf("FAIL umid_encoder_count: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_encoder_count: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_encoder_position_rad(((void*)workspace.bytes)) != 0) {
        printf("FAIL umid_encoder_position_rad: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_encoder_position_rad: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_encoder_velocity_rps(((void*)workspace.bytes)) != 0) {
        printf("FAIL umid_encoder_velocity_rps: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_encoder_velocity_rps: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    umid_encoder_reset(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_encoder_reset: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_gnss_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_gnss_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_gnss_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    umid_gnss_process(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_gnss_process: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_gnss_has_fix(((void*)workspace.bytes)) != false) {
        printf("FAIL umid_gnss_has_fix: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_gnss_has_fix: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_imu_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_imu_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_imu_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_imu_start(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_imu_start: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_imu_start: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    umid_imu_stop(((void*)workspace.bytes));
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_imu_stop: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_imu_read_blocking(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_imu_read_blocking: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_imu_read_blocking: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_magnetometer_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_magnetometer_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_magnetometer_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_magnetometer_read(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_magnetometer_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_magnetometer_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_pressure_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_pressure_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_pressure_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_pressure_read(((void*)workspace.bytes), ((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_pressure_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_pressure_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_range_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_range_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_range_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_range_read(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_range_read: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_range_read: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_temperature_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_temperature_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_temperature_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_temperature_read_degC(((void*)workspace.bytes)) != 0) {
        printf("FAIL umid_temperature_read_degC: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_temperature_read_degC: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_voltage_init(((void*)workspace.bytes)) != UETD_NOT_SUPPORTED) {
        printf("FAIL umid_voltage_init: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_voltage_init: changed caller-owned storage\n");
        ++failures;
    }
    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));
    memcpy(before, workspace.bytes, sizeof(before));
    if (umid_voltage_read_v(((void*)workspace.bytes)) != 0) {
        printf("FAIL umid_voltage_read_v: unexpected fail-closed return\n");
        ++failures;
    }
    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {
        printf("FAIL umid_voltage_read_v: changed caller-owned storage\n");
        ++failures;
    }
    if (failures != 0u) {
        printf("%zu uetd_umid_scaffold_contracts check(s) failed across 25 operations\n", failures);
        return 1;
    }
    printf("Validated 25 uetd_umid_scaffold_contracts operations from 9 source files\n");
    return 0;
}
