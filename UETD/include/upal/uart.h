/// @file include/upal/uart.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_UART_H
#define UPAL_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include "upal/dma.h"
#include <uhal/target.h>

typedef struct {
    void* instance; /* USART1, USART2, LPUART1, etc. */
    upal_dma_t* dma_tx;
    upal_dma_t* dma_rx;
    uetd_u32_t dma_tx_request;
    uetd_u32_t dma_rx_request;
    IRQn_Type irqn; /* UART global IRQ (for IDLE line) */
    uetd_u8_t irq_priority;
    uhal_gpio_pin_t tx_pin, rx_pin;
    uetd_u8_t tx_af, rx_af;
} upal_uart_hw_t;

typedef struct {
    const upal_uart_hw_t* hw;
    uetd_u32_t baud;
    uetd_u8_t* rx_buf; /* circular DMA ring buffer */
    uetd_u32_t rx_buf_size;
    volatile uetd_u32_t rx_head; /* updated in IDLE/DMA ISR */
    upal_dma_callback_t tx_callback;
    void* tx_ctx;
    volatile uetd_u32_t state;
} upal_uart_t;

/* Initialise — starts circular DMA RX immediately */
uetd_status_t upal_uart_init(upal_uart_t* u, const upal_uart_hw_t* hw, uetd_u32_t baud, uetd_u8_t* rx_buf, uetd_u32_t rx_buf_size);

/* Non-blocking DMA transmit */
uetd_status_t upal_uart_tx_dma(upal_uart_t* u, const uetd_u8_t* data, uetd_u32_t len, upal_dma_callback_t cb, void* ctx);

/* Blocking transmit — init sequences, fault logging only */
uetd_status_t upal_uart_tx_blocking(upal_uart_t* u, const uetd_u8_t* data, uetd_u32_t len, uetd_u32_t timeout_ms);

/* Circular DMA receive — call from IDLE ISR or DMA HT/TC ISR */
uetd_u32_t upal_uart_rx_available(const upal_uart_t* u);
uetd_u32_t upal_uart_rx_read(upal_uart_t* u, uetd_u8_t* dst, uetd_u32_t max_len);

/* ISR entry points */
void upal_uart_irq_handler(upal_uart_t* u);    /* IDLE line */
void upal_uart_dma_rx_handler(upal_uart_t* u); /* DMA HT/TC */
void upal_uart_dma_tx_handler(upal_uart_t* u);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_UART_H */
