/// @file src/umid/temp.c
/// @brief Source scaffold for the V1.2 public contract in uef/umid/umid_temperature.h.
///
/// Implementation intent: Keep thermocouple, thermistor, RTD, and MCU-temperature conversion
///   paths distinct and unit-labeled.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/umid/umid_temperature.h>

uef_status_t umid_temperature_init(
    umid_temperature_t* ts
) {
    /* TODO(UEF UMID temperature init):
     * Validate ADC channel and sensor-kind parameters, load the selected thermocouple,
     * thermistor, RTD, or MCU-temperature model, and establish its calibration and valid range.
     */
    (void)ts;
    return UEF_NOT_SUPPORTED;
}

uef_f32_t umid_temperature_read_degC(
    umid_temperature_t* ts
) {
    /* TODO(UEF UMID temperature read):
     * Fetch a fresh ADC sample, convert it through the selected sensor model to degrees Celsius,
     * and apply calibration. Because this getter returns only a float, define a separate validity
     * path before using zero as anything other than its current neutral scaffold value.
     */
    (void)ts;
    return 0;
}
