/// @file include/uef/upal/upal_i2c.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_I2C_H
#define UPAL_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_dma.h"
#include "uef/uhal/uhal_gpio.h"
#include <uef/uhal/target.h>

typedef enum {
    UPAL_I2C_SPEED_STANDARD   = 100000,
    UPAL_I2C_SPEED_FAST       = 400000,
    UPAL_I2C_SPEED_FAST_PLUS  = 1000000,
} upal_i2c_speed_t;

typedef struct {
    void*            instance;
    upal_dma_t*      dma_tx;
    upal_dma_t*      dma_rx;
    IRQn_Type        ev_irqn;   /* event interrupt */
    IRQn_Type        er_irqn;   /* error interrupt */
    uhal_gpio_pin_t  scl_pin, sda_pin;
    uef_u8_t         scl_af,  sda_af;
} upal_i2c_hw_t;

typedef struct {
    const upal_i2c_hw_t*  hw;
    upal_i2c_speed_t      speed;
    volatile uef_u32_t    state;
} upal_i2c_t;

uef_status_t upal_i2c_init(upal_i2c_t* i, const upal_i2c_hw_t* hw,
                             upal_i2c_speed_t speed);

uef_status_t upal_i2c_write_dma(upal_i2c_t* i, uef_u8_t addr,
                                  const uef_u8_t* data, uef_u32_t len,
                                  upal_dma_callback_t cb, void* ctx);

uef_status_t upal_i2c_read_dma(upal_i2c_t* i, uef_u8_t addr,
                                 uef_u8_t* data, uef_u32_t len,
                                 upal_dma_callback_t cb, void* ctx);

/* Write register then read — single combined transaction (common for IMUs) */
uef_status_t upal_i2c_write_read_dma(upal_i2c_t* i, uef_u8_t addr,
                                       const uef_u8_t* tx, uef_u32_t tx_len,
                                       uef_u8_t* rx,       uef_u32_t rx_len,
                                       upal_dma_callback_t cb, void* ctx);

uef_status_t upal_i2c_write_blocking(upal_i2c_t* i, uef_u8_t addr,
                                       const uef_u8_t* data, uef_u32_t len,
                                       uef_u32_t timeout_ms);
uef_status_t upal_i2c_read_blocking(upal_i2c_t* i, uef_u8_t addr,
                                      uef_u8_t* data, uef_u32_t len,
                                      uef_u32_t timeout_ms);

void upal_i2c_ev_irq_handler(upal_i2c_t* i);
void upal_i2c_er_irq_handler(upal_i2c_t* i);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_I2C_H */
