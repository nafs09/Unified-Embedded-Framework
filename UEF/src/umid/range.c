/// @file src/umid/range.c
/// @brief Source scaffold for the V1.1 public contract in uef/umid/umid_range.h.
///
/// Implementation intent: Normalize distance samples and reject invalid or stale sensor
///   readings.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_range.h>

uef_status_t umid_range_init(
    umid_range_t* r
) {
    /* TODO(UEF UMID range init):
     * Validate transport, range limits, and conversion mode; configure the sensor and initialize
     * validity/timestamp state without marking an old power-on register value as a sample.
     */
    (void)r;
    return UEF_NOT_SUPPORTED;
}

uef_status_t umid_range_read(
    umid_range_t* r
) {
    /* TODO(UEF UMID range read):
     * Start or collect one conversion according to the device mode, bound its ready wait, reject
     * invalid/out-of-range status, and publish distance plus acquisition time only on success.
     */
    (void)r;
    return UEF_NOT_SUPPORTED;
}
