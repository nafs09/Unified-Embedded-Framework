/// @file src/umid/drivers/pressure.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/pressure.h.
///
/// Implementation intent: Decode the selected sensor format and apply only documented
///   compensation coefficients.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/pressure.h>

uetd_status_t umid_pressure_init(umid_pressure_t* ps) {
    /* TODO(UETD UMID pressure init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete barometer manually, use UPAL for hardware access, and select its source through
     * the owning module manifest when generator support is added. Do not add a device registry;
     * NEXUS supplies simulated signals with stochastic noise/bias random walk or simulation-data
     * replay; do not implement simulation here.
     * Validate the bus/device configuration and required calibration coefficients, then run
     * the explicitly selected part's documented startup sequence. Establish pressure and
     * temperature units and conversion readiness before accepting reads.
     */
    (void)ps;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_pressure_read(umid_pressure_t* ps, umid_pressure_data_t* out) {
    /* TODO(UETD UMID pressure read):
     * Validate pointers and ready state, obtain a complete raw pressure/temperature sample,
     * apply only the selected sensor's documented compensation, timestamp the result, and leave
     * caller storage unchanged if transport or range checks fail.
     */
    (void)ps;
    (void)out;
    return UETD_NOT_SUPPORTED;
}
