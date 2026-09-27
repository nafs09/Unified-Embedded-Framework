/// @file include/uef/umid/umid_magnetometer.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UMID_MAGNETOMETER_H
#define UMID_MAGNETOMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_i2c.h"

typedef struct {
    uef_f32_t x_uT, y_uT, z_uT;
    uef_u64_t timestamp_us;
} umid_mag_data_t;

typedef struct {
    upal_i2c_t* i2c;
    uef_u8_t    addr;
    uef_f32_t   hard_iron[3];    /* calibration offsets */
    uef_f32_t   soft_iron[9];    /* 3×3 scale/cross-axis correction */
} umid_magnetometer_t;

uef_status_t umid_magnetometer_init(umid_magnetometer_t* m);
uef_status_t umid_magnetometer_read(umid_magnetometer_t* m,
                                      umid_mag_data_t* out);

#ifdef __cplusplus
}
#endif

#endif /* UMID_MAGNETOMETER_H */
