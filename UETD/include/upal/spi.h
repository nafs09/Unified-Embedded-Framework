/// @file include/upal/spi.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_SPI_H
#define UPAL_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include "upal/dma.h"
#include <uhal/target.h>

typedef enum {
    UPAL_SPI_MODE_0 = 0, /* CPOL=0 CPHA=0 */
    UPAL_SPI_MODE_1 = 1, /* CPOL=0 CPHA=1 */
    UPAL_SPI_MODE_2 = 2, /* CPOL=1 CPHA=0 */
    UPAL_SPI_MODE_3 = 3, /* CPOL=1 CPHA=1 */
} upal_spi_mode_t;

typedef struct {
    void* instance;
    upal_dma_t* dma_tx;
    upal_dma_t* dma_rx;
    IRQn_Type irqn;
    uhal_gpio_pin_t sck_pin, mosi_pin, miso_pin;
    uetd_u8_t sck_af, mosi_af, miso_af;
} upal_spi_hw_t;

typedef struct {
    const upal_spi_hw_t* hw;
    uetd_u32_t clock_hz;
    upal_spi_mode_t mode;
    uetd_u8_t data_bits; /* 8 or 16 */
    bool msb_first;
    volatile uetd_u32_t state;
} upal_spi_t;

uetd_status_t upal_spi_init(upal_spi_t* s, const upal_spi_hw_t* hw, uetd_u32_t clock_hz, upal_spi_mode_t mode);

/* Full-duplex DMA transfer. tx or rx may be NULL for simplex operation. */
uetd_status_t upal_spi_transfer_dma(upal_spi_t* s, uhal_gpio_pin_t cs_pin, const uetd_u8_t* tx, uetd_u8_t* rx, uetd_u32_t len,
                                   upal_dma_callback_t cb, void* ctx);

uetd_status_t upal_spi_transfer_blocking(upal_spi_t* s, uhal_gpio_pin_t cs_pin, const uetd_u8_t* tx, uetd_u8_t* rx, uetd_u32_t len,
                                        uetd_u32_t timeout_ms);

void upal_spi_dma_handler(upal_spi_t* s);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_SPI_H */
