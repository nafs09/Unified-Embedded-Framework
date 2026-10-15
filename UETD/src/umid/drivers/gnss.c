/// @file src/umid/drivers/gnss.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/gnss.h.
///
/// Implementation intent: Parse bounded NMEA/UBX frames incrementally and publish a fix only
///   after checksum and field validation.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/gnss.h>

uetd_status_t umid_gnss_init(umid_gnss_t* g) {
    /* TODO(UETD UMID GNSS init):
     * This is the generic UMID contract scaffold, not receiver-specific support. Add each
     * concrete receiver/parser binding manually, use UPAL UART, and select its source through the
     * owning module manifest when generator support is added. Do not add a device registry;
     * NEXUS supplies simulated signals with stochastic noise/bias random walk or simulation-data
     * replay; do not implement simulation here.
     * Validate UART and caller-owned parse buffer/capacity, clear parser state, and choose the
     * explicitly configured NMEA/UBX framing. Establish callback ownership and a maximum frame
     * size; do not probe for a receiver at runtime.
     */
    (void)g;
    return UETD_NOT_SUPPORTED;
}

void umid_gnss_process(umid_gnss_t* g) {
    /* TODO(UETD UMID GNSS process):
     * Consume only available UART bytes within a bounded budget; parse supported NMEA/UBX frames
     * without allocation; validate checksums, field ranges, and fix quality; then update the
     * complete fix/timestamp atomically and invoke `on_fix` in its documented safe context.
     */
    (void)g;
}

bool umid_gnss_has_fix(const umid_gnss_t* g) {
    /* TODO(UETD UMID GNSS has-fix):
     * Return true only for a validated nonzero fix type whose timestamp is within the declared
     * stale-data window. Keep fix-validity separate from the receiver's last status byte.
     */
    (void)g;
    return false;
}
