/// @file src/umid/drivers/voltage.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/voltage.h.
///
/// Implementation intent: Convert ADC values through the divider ratio and reference voltage,
///   preserving calibrated units.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/voltage.h>

uetd_status_t umid_voltage_init(umid_voltage_t* vs) {
    /* TODO(UETD UMID voltage init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete board/sensor binding manually, use UPAL for hardware access, and select its source
     * through the owning module manifest when generator support is added. Do not add a device
     * registry. NEXUS supplies simulated signals with stochastic noise/bias random walk or
     * simulation-data replay; do not implement simulation here.
     * Validate the ADC source, reference voltage, divider ratio, and calibration range; define
     * who starts the ADC channel and how sample freshness is tracked.
     */
    (void)vs;
    return UETD_NOT_SUPPORTED;
}

uetd_f32_t umid_voltage_read_v(const umid_voltage_t* vs) {
    /* TODO(UETD UMID voltage read):
     * Snapshot a fresh ADC result, convert through reference voltage and divider ratio to volts,
     * then apply calibration. Define how invalid/stale reads are surfaced because this scalar
     * getter currently cannot distinguish them from the neutral zero return.
     */
    (void)vs;
    return 0;
}
