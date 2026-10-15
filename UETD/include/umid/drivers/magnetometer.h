/// @file include/umid/drivers/magnetometer.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_MAGNETOMETER_H
#define UMID_MAGNETOMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/i2c.h"

typedef struct {
    uetd_f32_t x_uT, y_uT, z_uT;
    uetd_u64_t timestamp_us;
} umid_mag_data_t;

typedef struct {
    upal_i2c_t* i2c;
    uetd_u8_t addr;
    uetd_f32_t hard_iron[3]; /* calibration offsets */
    uetd_f32_t soft_iron[9]; /* 3×3 scale/cross-axis correction */
} umid_magnetometer_t;

uetd_status_t umid_magnetometer_init(umid_magnetometer_t* m);
uetd_status_t umid_magnetometer_read(umid_magnetometer_t* m, umid_mag_data_t* out);

#ifdef __cplusplus
}
#endif

#endif /* UMID_MAGNETOMETER_H */
