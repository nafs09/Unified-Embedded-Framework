/// @file include/uef/upal/upal_can.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_CAN_H
#define UPAL_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/uhal/uhal_gpio.h"
#include <uef/uhal/target.h>

typedef struct {
    uef_u32_t id;
    bool      extended;   /* 29-bit vs 11-bit */
    bool      remote;
    bool      fd;         /* CAN-FD */
    bool      brs;        /* bit rate switch */
    uef_u8_t  dlc;
    uef_u8_t  data[64];  /* max 64 bytes CAN-FD */
} upal_can_frame_t;

typedef struct {
    void*            instance;     /* FDCAN1, FDCAN2, bxCAN */
    IRQn_Type        rx_irqn;
    IRQn_Type        tx_irqn;
    IRQn_Type        err_irqn;
    uhal_gpio_pin_t  tx_pin, rx_pin;
    uef_u8_t         tx_af,  rx_af;
} upal_can_hw_t;

typedef struct {
    uef_u32_t nominal_baud;
    uef_u32_t data_baud;    /* CAN-FD data phase; 0 = use nominal */
    bool      fd_mode;
    bool      loopback;
    bool      silent;
} upal_can_cfg_t;

typedef void (*upal_can_rx_cb_t)(const upal_can_frame_t* frame, void* ctx);

typedef struct {
    const upal_can_hw_t*  hw;
    upal_can_cfg_t        cfg;
    upal_can_rx_cb_t      rx_cb;
    void*                 rx_ctx;
    volatile uef_u32_t    state;
} upal_can_t;

uef_status_t upal_can_init(upal_can_t* c, const upal_can_hw_t* hw,
                             const upal_can_cfg_t* cfg,
                             upal_can_rx_cb_t rx_cb, void* rx_ctx);

/* Hardware ID/mask filter */
uef_status_t upal_can_add_filter(upal_can_t* c, uef_u32_t id,
                                   uef_u32_t mask, bool extended);

uef_status_t upal_can_transmit(upal_can_t* c, const upal_can_frame_t* f,
                                 uef_u32_t timeout_ms);
uef_status_t upal_can_transmit_async(upal_can_t* c,
                                       const upal_can_frame_t* f);

void upal_can_rx_irq_handler(upal_can_t* c);
void upal_can_tx_irq_handler(upal_can_t* c);
void upal_can_err_irq_handler(upal_can_t* c);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_CAN_H */
