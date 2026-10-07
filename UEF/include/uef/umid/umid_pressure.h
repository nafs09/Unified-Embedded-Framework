/// @file include/uef/umid/umid_pressure.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UMID_PRESSURE_H
#define UMID_PRESSURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_spi.h"
#include "uef/upal/upal_i2c.h"

typedef struct {
    /* Bus config — same union pattern as umid_imu_t */
    uef_f32_t pressure_pa;
    uef_f32_t temperature_degC;
    uef_u64_t timestamp_us;
} umid_pressure_data_t;

typedef struct {
    upal_spi_t*   spi;
    uhal_gpio_pin_t cs;
    /* Device-specific calibration coefficients stored here */
    uef_u32_t     calib[12];
} umid_pressure_t;

uef_status_t umid_pressure_init(umid_pressure_t* ps);
uef_status_t umid_pressure_read(umid_pressure_t* ps,
                                  umid_pressure_data_t* out);

#ifdef __cplusplus
}
#endif

#endif /* UMID_PRESSURE_H */
