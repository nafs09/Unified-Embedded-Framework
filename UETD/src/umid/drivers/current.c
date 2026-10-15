/// @file src/umid/drivers/current.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/current.h.
///
/// Implementation intent: Convert ADC readings with the configured shunt/gain calibration and
///   make zero-offset calibration explicit.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/current.h>

uetd_status_t umid_current_init(umid_current_t* cs) {
    /* TODO(UETD UMID current init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete device manually, use UPAL for hardware access, and select its source through the
     * owning module manifest when generator support is added. Do not add a device registry;
     * NEXUS supplies simulated signals with stochastic noise/bias random walk or simulation-data
     * replay; do not implement simulation here.
     * Validate ADC channel and finite scale/shunt parameters, initialize the calibration state,
     * and define whether this module owns ADC startup or consumes an already-running channel.
     */
    (void)cs;
    return UETD_NOT_SUPPORTED;
}

uetd_f32_t umid_current_read_a(const umid_current_t* cs) {
    /* TODO(UETD UMID current read):
     * Fetch or snapshot the latest ADC result, subtract the calibrated zero offset, apply shunt
     * and gain scaling to amperes, and define stale/invalid-sample reporting. If zero is an
     * ambiguous valid reading, extend the API to return status separately from the value.
     */
    (void)cs;
    return 0;
}

uetd_status_t umid_current_calibrate_zero(umid_current_t* cs) {
    /* TODO(UETD UMID current calibration):
     * Gather a documented number of stationary samples, reject invalid/saturated readings,
     * compute the zero offset using bounded fixed-memory accumulation, and update calibration
     * atomically only after the full sample window succeeds.
     */
    (void)cs;
    return UETD_NOT_SUPPORTED;
}
