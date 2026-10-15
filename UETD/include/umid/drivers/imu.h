/// @file include/umid/drivers/imu.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UMID_IMU_H
#define UMID_IMU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/i2c.h"
#include "upal/spi.h"

/* Calibrated IMU data — always in SI units */
typedef struct {
    uetd_f32_t accel_x_mps2;
    uetd_f32_t accel_y_mps2;
    uetd_f32_t accel_z_mps2;
    uetd_f32_t gyro_x_rps;
    uetd_f32_t gyro_y_rps;
    uetd_f32_t gyro_z_rps;
    uetd_f32_t temp_degC;
    uetd_u64_t timestamp_us;
} umid_imu_data_t;

typedef enum {
    UMID_IMU_BUS_SPI,
    UMID_IMU_BUS_I2C,
} umid_imu_bus_t;

/* Generic IMU driver — device-specific init registered separately */
typedef struct {
    umid_imu_bus_t bus_type;
    union {
        struct {
            upal_spi_t* spi;
            uhal_gpio_pin_t cs;
        } spi;
        struct {
            upal_i2c_t* i2c;
            uetd_u8_t addr;
        } i2c;
    } bus;
    /* Calibration */
    uetd_f32_t accel_scale; /* LSB to m/s² */
    uetd_f32_t gyro_scale;  /* LSB to rad/s */
    uetd_f32_t accel_bias[3];
    uetd_f32_t gyro_bias[3];
    uetd_f32_t gyro_misalignment[9]; /* 3×3 misalignment correction */
    /* DMA receive buffer — raw register values */
    uetd_u8_t* dma_buf;
    uetd_u32_t dma_buf_len;
    /* Callback on new data (from DMA complete ISR) */
    void (*on_data)(umid_imu_data_t* data, void* ctx);
    void* on_data_ctx;
} umid_imu_t;

uetd_status_t umid_imu_init(umid_imu_t* imu);
uetd_status_t umid_imu_start(umid_imu_t* imu);
void umid_imu_stop(umid_imu_t* imu);
uetd_status_t umid_imu_read_blocking(umid_imu_t* imu, umid_imu_data_t* out);

/* Device-specific init functions (registered per driver) */
/* Examples: umid_imu_init_icm42688p, umid_imu_init_bmi088 */

#ifdef __cplusplus
}
#endif

#endif /* UMID_IMU_H */
