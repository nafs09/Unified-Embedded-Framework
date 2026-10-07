/// @file include/uef/umid/umid_imu.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UMID_IMU_H
#define UMID_IMU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_spi.h"
#include "uef/upal/upal_i2c.h"

/* Calibrated IMU data — always in SI units */
typedef struct {
    uef_f32_t accel_x_mps2;
    uef_f32_t accel_y_mps2;
    uef_f32_t accel_z_mps2;
    uef_f32_t gyro_x_rps;
    uef_f32_t gyro_y_rps;
    uef_f32_t gyro_z_rps;
    uef_f32_t temp_degC;
    uef_u64_t timestamp_us;
} umid_imu_data_t;

typedef enum {
    UMID_IMU_BUS_SPI,
    UMID_IMU_BUS_I2C,
} umid_imu_bus_t;

/* Generic IMU driver — device-specific init registered separately */
typedef struct {
    umid_imu_bus_t  bus_type;
    union {
        struct { upal_spi_t* spi; uhal_gpio_pin_t cs; } spi;
        struct { upal_i2c_t* i2c; uef_u8_t addr; }     i2c;
    } bus;
    /* Calibration */
    uef_f32_t accel_scale;      /* LSB to m/s² */
    uef_f32_t gyro_scale;       /* LSB to rad/s */
    uef_f32_t accel_bias[3];
    uef_f32_t gyro_bias[3];
    uef_f32_t gyro_misalignment[9]; /* 3×3 misalignment correction */
    /* DMA receive buffer — raw register values */
    uef_u8_t* dma_buf;
    uef_u32_t dma_buf_len;
    /* Callback on new data (from DMA complete ISR) */
    void (*on_data)(umid_imu_data_t* data, void* ctx);
    void* on_data_ctx;
} umid_imu_t;

uef_status_t umid_imu_init(umid_imu_t* imu);
uef_status_t umid_imu_start(umid_imu_t* imu);
void         umid_imu_stop(umid_imu_t* imu);
uef_status_t umid_imu_read_blocking(umid_imu_t* imu, umid_imu_data_t* out);

/* Device-specific init functions (registered per driver) */
/* Examples: umid_imu_init_icm42688p, umid_imu_init_bmi088 */

#ifdef __cplusplus
}
#endif

#endif /* UMID_IMU_H */
