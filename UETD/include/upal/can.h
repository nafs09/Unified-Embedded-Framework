/// @file include/upal/can.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_CAN_H
#define UPAL_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include <uhal/target.h>

typedef struct {
    uetd_u32_t id;
    bool extended; /* 29-bit vs 11-bit */
    bool remote;
    bool fd;  /* CAN-FD */
    bool brs; /* bit rate switch */
    uetd_u8_t dlc;
    uetd_u8_t data[64]; /* max 64 bytes CAN-FD */
} upal_can_frame_t;

typedef struct {
    void* instance; /* FDCAN1, FDCAN2, bxCAN */
    IRQn_Type rx_irqn;
    IRQn_Type tx_irqn;
    IRQn_Type err_irqn;
    uhal_gpio_pin_t tx_pin, rx_pin;
    uetd_u8_t tx_af, rx_af;
} upal_can_hw_t;

typedef struct {
    uetd_u32_t nominal_baud;
    uetd_u32_t data_baud; /* CAN-FD data phase; 0 = use nominal */
    bool fd_mode;
    bool loopback;
    bool silent;
} upal_can_cfg_t;

typedef void (*upal_can_rx_cb_t)(const upal_can_frame_t* frame, void* ctx);

typedef struct {
    const upal_can_hw_t* hw;
    upal_can_cfg_t cfg;
    upal_can_rx_cb_t rx_cb;
    void* rx_ctx;
    volatile uetd_u32_t state;
} upal_can_t;

uetd_status_t upal_can_init(upal_can_t* c, const upal_can_hw_t* hw, const upal_can_cfg_t* cfg, upal_can_rx_cb_t rx_cb,
                           void* rx_ctx);

/* Hardware ID/mask filter */
uetd_status_t upal_can_add_filter(upal_can_t* c, uetd_u32_t id, uetd_u32_t mask, bool extended);

uetd_status_t upal_can_transmit(upal_can_t* c, const upal_can_frame_t* f, uetd_u32_t timeout_ms);
uetd_status_t upal_can_transmit_async(upal_can_t* c, const upal_can_frame_t* f);

void upal_can_rx_irq_handler(upal_can_t* c);
void upal_can_tx_irq_handler(upal_can_t* c);
void upal_can_err_irq_handler(upal_can_t* c);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_CAN_H */
