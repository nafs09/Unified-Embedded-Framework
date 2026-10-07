/// @file src/umid/gnss.c
/// @brief Source scaffold for the V1.2 public contract in uef/umid/umid_gnss.h.
///
/// Implementation intent: Parse bounded NMEA/UBX frames incrementally and publish a fix only
///   after checksum and field validation.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_gnss.h>

uef_status_t umid_gnss_init(
    umid_gnss_t* g
) {
    /* TODO(UEF UMID GNSS init):
     * Validate UART and caller-owned parse buffer/capacity, clear parser state, and choose the
     * supported receiver framing driver. Establish callback ownership and a maximum frame size.
     */
    (void)g;
    return UEF_NOT_SUPPORTED;
}

void umid_gnss_process(
    umid_gnss_t* g
) {
    /* TODO(UEF UMID GNSS process):
     * Consume only available UART bytes within a bounded budget; parse supported NMEA/UBX frames
     * without allocation; validate checksums, field ranges, and fix quality; then update the
     * complete fix/timestamp atomically and invoke `on_fix` in its documented safe context.
     */
    (void)g;
}

bool umid_gnss_has_fix(
    const umid_gnss_t* g
) {
    /* TODO(UEF UMID GNSS has-fix):
     * Return true only for a validated nonzero fix type whose timestamp is within the declared
     * stale-data window. Keep fix-validity separate from the receiver's last status byte.
     */
    (void)g;
    return false;
}
