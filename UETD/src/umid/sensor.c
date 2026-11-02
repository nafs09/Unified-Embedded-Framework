/// @file src/umid/sensor.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "umid/sensor.h"

uetd_status_t umid_sensor_init(umid_sensor_t* sensor, const umid_sensor_config_t* config) {
    /* TODO: Bind device/provider identity, typed value schema, units/frame and sample/calibration contract. */
    (void)sensor;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_sensor_poll(umid_sensor_t* sensor, uetd_u16_t work_budget) {
    /* TODO: Bound acquisition/recovery work and preserve capture time rather than replacing it with parse time. */
    (void)sensor;
    (void)work_budget;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_sensor_read(umid_sensor_t* sensor, ucore_write_span_t value, umid_sample_metadata_t* metadata) {
    /* TODO: Publish the complete value and metadata together; distinguish stale, missing, invalid and overrun samples. */
    (void)sensor;
    (void)value;
    (void)metadata;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_sensor_activate_calibration(umid_sensor_t* sensor, uetd_u32_t generation) {
    /* TODO: Validate compatible device/schema calibration and activate it only at the declared sample boundary. */
    (void)sensor;
    (void)generation;
    return UETD_NOT_SUPPORTED;
}

