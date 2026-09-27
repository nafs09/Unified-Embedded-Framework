/// @file src/upal/opamp/opamp.c
/// @brief Source scaffold for the V1.1 public contract in uef/upal/upal_opamp.h.
///
/// Implementation intent: Validate PGA/follower configuration and calibration before enabling
///   the selected internal amplifier.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/upal/upal_opamp.h>

uef_status_t upal_opamp_init(
    upal_opamp_t* o,
    const upal_opamp_hw_t* hw,
    upal_opamp_mode_t mode,
    uef_u8_t pga_gain
) {
    /* TODO(UEF UPAL OPAMP):
     * Validate all descriptors and configuration, enable/reset the peripheral in the
     * required order, and publish a ready state only after setup succeeds. Validate
     * mode/gain against target capabilities, respect startup settling time, and propagate
     * calibration failure.
     */
    (void)o;
    (void)hw;
    (void)mode;
    (void)pga_gain;
    return UEF_NOT_SUPPORTED;
}

void upal_opamp_enable(
    upal_opamp_t* o
) {
    /* TODO(UEF UPAL OPAMP):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Validate mode/gain
     * against target capabilities, respect startup settling time, and propagate calibration
     * failure.
     */
    (void)o;
}

void upal_opamp_disable(
    upal_opamp_t* o
) {
    /* TODO(UEF UPAL OPAMP):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Validate mode/gain
     * against target capabilities, respect startup settling time, and propagate calibration
     * failure.
     */
    (void)o;
}

uef_status_t upal_opamp_calibrate(
    upal_opamp_t* o
) {
    /* TODO(UEF UPAL OPAMP):
     * Implement the declared operation with argument/state validation, bounded waiting,
     * precise status propagation, and documented callback/ISR ownership. Validate mode/gain
     * against target capabilities, respect startup settling time, and propagate calibration
     * failure.
     */
    (void)o;
    return UEF_NOT_SUPPORTED;
}
