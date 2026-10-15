/// @file src/umid/drivers/temp.c
/// @brief Source scaffold for the V1.2 public contract in umid/drivers/temperature.h.
///
/// Implementation intent: Keep thermocouple, thermistor, RTD, and MCU-temperature conversion
///   paths distinct and unit-labeled.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <umid/drivers/temperature.h>

uetd_status_t umid_temperature_init(umid_temperature_t* ts) {
    /* TODO(UETD UMID temperature init):
     * This is the generic UMID contract scaffold, not physical-device support. Add each
     * concrete sensor/board binding manually, use UPAL for hardware access, and select its source
     * through the owning module manifest when generator support is added. Do not add a device
     * registry. NEXUS supplies simulated signals with stochastic noise/bias random walk or
     * simulation-data replay; do not implement simulation here.
     * Validate ADC channel and sensor-kind parameters, load the selected thermocouple,
     * thermistor, RTD, or MCU-temperature model, and establish its calibration and valid range.
     */
    (void)ts;
    return UETD_NOT_SUPPORTED;
}

uetd_f32_t umid_temperature_read_degC(umid_temperature_t* ts) {
    /* TODO(UETD UMID temperature read):
     * Fetch a fresh ADC sample, convert it through the selected sensor model to degrees Celsius,
     * and apply calibration. Because this getter returns only a float, define a separate validity
     * path before using zero as anything other than its current neutral scaffold value.
     */
    (void)ts;
    return 0;
}
