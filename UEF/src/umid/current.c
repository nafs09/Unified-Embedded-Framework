/// @file src/umid/current.c
/// @brief Source scaffold for the V1.1 public contract in uef/umid/umid_current.h.
///
/// Implementation intent: Convert ADC readings with the configured shunt/gain calibration and
///   make zero-offset calibration explicit.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_current.h>

uef_status_t umid_current_init(
    umid_current_t* cs
) {
    /* TODO(UEF UMID current init):
     * Validate ADC channel and finite scale/shunt parameters, initialize the calibration state,
     * and define whether this module owns ADC startup or consumes an already-running channel.
     */
    (void)cs;
    return UEF_NOT_SUPPORTED;
}

uef_f32_t umid_current_read_a(
    const umid_current_t* cs
) {
    /* TODO(UEF UMID current read):
     * Fetch or snapshot the latest ADC result, subtract the calibrated zero offset, apply shunt
     * and gain scaling to amperes, and define stale/invalid-sample reporting. If zero is an
     * ambiguous valid reading, extend the API to return status separately from the value.
     */
    (void)cs;
    return 0;
}

uef_status_t umid_current_calibrate_zero(
    umid_current_t* cs
) {
    /* TODO(UEF UMID current calibration):
     * Gather a documented number of stationary samples, reject invalid/saturated readings,
     * compute the zero offset using bounded fixed-memory accumulation, and update calibration
     * atomically only after the full sample window succeeds.
     */
    (void)cs;
    return UEF_NOT_SUPPORTED;
}
