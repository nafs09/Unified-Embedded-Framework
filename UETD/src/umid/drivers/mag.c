/// @file src/umid/drivers/mag.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/magnetometer.h.
///
/// Implementation intent: Apply explicit hard-iron and soft-iron calibration without embedding
///   a guessed calibration model.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/magnetometer.h>

uetd_status_t umid_magnetometer_init(umid_magnetometer_t* m) {
    /* TODO(UETD UMID magnetometer init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete magnetometer manually, use UPAL for hardware access, and select its source through
     * the owning module manifest when generator support is added. Do not add a device registry;
     * NEXUS supplies simulated signals with stochastic noise/bias random walk or simulation-data
     * replay; do not implement simulation here.
     * Validate transport and finite calibration matrices, configure the selected sensor, and
     * establish the axis/sign convention. Keep hard-iron offset and soft-iron matrix explicit;
     * do not choose or synthesize calibration values here.
     */
    (void)m;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_magnetometer_read(umid_magnetometer_t* m, umid_mag_data_t* out) {
    /* TODO(UETD UMID magnetometer read):
     * Validate `m` and `out`, acquire one complete raw sample with its timestamp, apply the
     * configured hard/soft-iron calibration and axis convention, and preserve `out` on any
     * bus, range, or stale-sample failure.
     */
    (void)m;
    (void)out;
    return UETD_NOT_SUPPORTED;
}
