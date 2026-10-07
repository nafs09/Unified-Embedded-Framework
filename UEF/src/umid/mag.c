/// @file src/umid/mag.c
/// @brief Source scaffold for the V1.2 public contract in uef/umid/umid_magnetometer.h.
///
/// Implementation intent: Apply explicit hard-iron and soft-iron calibration without embedding
///   a guessed calibration model.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_magnetometer.h>

uef_status_t umid_magnetometer_init(
    umid_magnetometer_t* m
) {
    /* TODO(UEF UMID magnetometer init):
     * Validate transport and finite calibration matrices, configure the selected sensor, and
     * establish the axis/sign convention. Keep hard-iron offset and soft-iron matrix explicit;
     * do not choose or synthesize calibration values here.
     */
    (void)m;
    return UEF_NOT_SUPPORTED;
}

uef_status_t umid_magnetometer_read(
    umid_magnetometer_t* m,
    umid_mag_data_t* out
) {
    /* TODO(UEF UMID magnetometer read):
     * Validate `m` and `out`, acquire one complete raw sample with its timestamp, apply the
     * configured hard/soft-iron calibration and axis convention, and preserve `out` on any
     * bus, range, or stale-sample failure.
     */
    (void)m;
    (void)out;
    return UEF_NOT_SUPPORTED;
}
