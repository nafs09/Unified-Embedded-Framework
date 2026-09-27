/// @file src/umid/pressure.c
/// @brief Source scaffold for the V1.1 public contract in uef/umid/umid_pressure.h.
///
/// Implementation intent: Decode the selected sensor format and apply only documented
///   compensation coefficients.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_pressure.h>

uef_status_t umid_pressure_init(
    umid_pressure_t* ps
) {
    /* TODO(UEF UMID pressure init):
     * Validate the bus/device configuration and required calibration coefficients, then run
     * the sensor-specific startup sequence. Establish pressure/temperature units and conversion
     * readiness before accepting reads.
     */
    (void)ps;
    return UEF_NOT_SUPPORTED;
}

uef_status_t umid_pressure_read(
    umid_pressure_t* ps,
    umid_pressure_data_t* out
) {
    /* TODO(UEF UMID pressure read):
     * Validate pointers and ready state, obtain a complete raw pressure/temperature sample,
     * apply only the selected sensor's documented compensation, timestamp the result, and leave
     * caller storage unchanged if transport or range checks fail.
     */
    (void)ps;
    (void)out;
    return UEF_NOT_SUPPORTED;
}
