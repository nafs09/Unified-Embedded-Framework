/// @file include/umid/drivers/pressure.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_PRESSURE_H
#define UMID_PRESSURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/i2c.h"
#include "upal/spi.h"

typedef struct {
    /* Bus config — same union pattern as umid_imu_t */
    uetd_f32_t pressure_pa;
    uetd_f32_t temperature_degC;
    uetd_u64_t timestamp_us;
} umid_pressure_data_t;

typedef struct {
    upal_spi_t* spi;
    uhal_gpio_pin_t cs;
    /* Device-specific calibration coefficients stored here */
    uetd_u32_t calib[12];
} umid_pressure_t;

uetd_status_t umid_pressure_init(umid_pressure_t* ps);
uetd_status_t umid_pressure_read(umid_pressure_t* ps, umid_pressure_data_t* out);

#ifdef __cplusplus
}
#endif

#endif /* UMID_PRESSURE_H */
