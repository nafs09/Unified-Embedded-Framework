/// @file src/umid/voltage.c
/// @brief Source scaffold for the V1.2 public contract in uef/umid/umid_voltage.h.
///
/// Implementation intent: Convert ADC values through the divider ratio and reference voltage,
///   preserving calibrated units.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_voltage.h>

uef_status_t umid_voltage_init(
    umid_voltage_t* vs
) {
    /* TODO(UEF UMID voltage init):
     * Validate the ADC source, reference voltage, divider ratio, and calibration range; define
     * who starts the ADC channel and how sample freshness is tracked.
     */
    (void)vs;
    return UEF_NOT_SUPPORTED;
}

uef_f32_t umid_voltage_read_v(
    const umid_voltage_t* vs
) {
    /* TODO(UEF UMID voltage read):
     * Snapshot a fresh ADC result, convert through reference voltage and divider ratio to volts,
     * then apply calibration. Define how invalid/stale reads are surfaced because this scalar
     * getter currently cannot distinguish them from the neutral zero return.
     */
    (void)vs;
    return 0;
}
