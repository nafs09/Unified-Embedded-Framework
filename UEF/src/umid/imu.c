/// @file src/umid/imu.c
/// @brief Source scaffold for the V1.2 public contract in uef/umid/umid_imu.h.
///
/// Implementation intent: Implement sensor-specific register setup, calibration, and
///   timestamped samples; keep bus errors visible to callers.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_imu.h>

uef_status_t umid_imu_init(
    umid_imu_t* imu
) {
    /* TODO(UEF UMID IMU init):
     * Validate the selected SPI/I2C descriptor, scales, DMA buffer, and callback contract;
     * configure the selected device-specific driver and establish axis/sign conventions.
     * Do not report ready until register identity/configuration checks have succeeded.
     */
    (void)imu;
    return UEF_NOT_SUPPORTED;
}

uef_status_t umid_imu_start(
    umid_imu_t* imu
) {
    /* TODO(UEF UMID IMU start):
     * Arm the configured sampling path and DMA only after initialization, then make callback
     * ownership and sample cadence explicit. Roll back any partially armed bus/DMA state if
     * startup fails; repeated start calls must have a documented result.
     */
    (void)imu;
    return UEF_NOT_SUPPORTED;
}

void umid_imu_stop(
    umid_imu_t* imu
) {
    /* TODO(UEF UMID IMU stop):
     * Stop new conversions, abort or finish the in-flight transfer according to the driver
     * contract, and prevent callbacks from observing freed/reused state. Make stopping an
     * unstarted/already stopped device safe and deterministic.
     */
    (void)imu;
}

uef_status_t umid_imu_read_blocking(
    umid_imu_t* imu,
    umid_imu_data_t* out
) {
    /* TODO(UEF UMID IMU read):
     * Validate both pointers and initialized state, fetch one complete register sample with a
     * bounded transport timeout, apply scale, bias, and misalignment calibration, then publish
     * SI units plus the monotonic acquisition timestamp. Leave `out` unchanged on failure.
     */
    (void)imu;
    (void)out;
    return UEF_NOT_SUPPORTED;
}
