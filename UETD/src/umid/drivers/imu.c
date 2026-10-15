/// @file src/umid/drivers/imu.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/imu.h.
///
/// Implementation intent: Implement sensor-specific register setup, calibration, and
///   timestamped samples; keep bus errors visible to callers.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/imu.h>

uetd_status_t umid_imu_init(umid_imu_t* imu) {
    /* TODO(UETD UMID IMU init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete IMU manually, use UPAL for hardware access, and select its source through the
     * owning module manifest when generator support is added. Do not add a device registry;
     * NEXUS supplies simulated signals with stochastic noise/bias random walk or simulation-data
     * replay; do not implement simulation here.
     * Validate the selected SPI/I2C descriptor, scales, DMA buffer, and callback contract;
     * configure the explicitly selected part and establish axis/sign conventions. Do not report
     * ready until that part's documented identity/configuration checks have succeeded.
     */
    (void)imu;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_imu_start(umid_imu_t* imu) {
    /* TODO(UETD UMID IMU start):
     * Arm the configured sampling path and DMA only after initialization, then make callback
     * ownership and sample cadence explicit. Roll back any partially armed bus/DMA state if
     * startup fails; repeated start calls must have a documented result.
     */
    (void)imu;
    return UETD_NOT_SUPPORTED;
}

void umid_imu_stop(umid_imu_t* imu) {
    /* TODO(UETD UMID IMU stop):
     * Stop new conversions, abort or finish the in-flight transfer according to the driver
     * contract, and prevent callbacks from observing freed/reused state. Make stopping an
     * unstarted/already stopped device safe and deterministic.
     */
    (void)imu;
}

uetd_status_t umid_imu_read_blocking(umid_imu_t* imu, umid_imu_data_t* out) {
    /* TODO(UETD UMID IMU read):
     * Validate both pointers and initialized state, fetch one complete register sample with a
     * bounded transport timeout, apply scale, bias, and misalignment calibration, then publish
     * SI units plus the monotonic acquisition timestamp. Leave `out` unchanged on failure.
     */
    (void)imu;
    (void)out;
    return UETD_NOT_SUPPORTED;
}
