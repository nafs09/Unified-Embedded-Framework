/// @file src/umid/drivers/range.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/range.h.
///
/// Implementation intent: Normalize distance samples and reject invalid or stale sensor
///   readings.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/range.h>

uetd_status_t umid_range_init(umid_range_t* r) {
    /* TODO(UETD UMID range init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete range sensor manually, use UPAL for hardware access, and select its source through
     * the owning module manifest when generator support is added. Do not add a device registry;
     * NEXUS supplies simulated signals with stochastic noise/bias random walk or simulation-data
     * replay; do not implement simulation here.
     * Validate transport, range limits, and conversion mode; configure the sensor and initialize
     * validity/timestamp state without marking an old power-on register value as a sample.
     */
    (void)r;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_range_read(umid_range_t* r) {
    /* TODO(UETD UMID range read):
     * Start or collect one conversion according to the device mode, bound its ready wait, reject
     * invalid/out-of-range status, and publish distance plus acquisition time only on success.
     */
    (void)r;
    return UETD_NOT_SUPPORTED;
}
