/// @file include/upal/qspi.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_QSPI_H
#define UPAL_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/gpio.h"
#include "upal/dma.h"

typedef struct {
    void* instance;
    upal_dma_t* dma;
    uetd_u32_t dma_request;
    uhal_gpio_pin_t clk_pin, ncs_pin;
    uhal_gpio_pin_t io0_pin, io1_pin, io2_pin, io3_pin;
    uetd_u8_t af;
    uetd_u32_t flash_size_bytes;
    uetd_u32_t max_clock_hz;
} upal_qspi_hw_t;

typedef struct {
    const upal_qspi_hw_t* hw;
    volatile uetd_u32_t state;
} upal_qspi_t;

uetd_status_t upal_qspi_init(upal_qspi_t* q, const upal_qspi_hw_t* hw);

/* Memory-mapped mode: flash at QUADSPI base address */
uetd_status_t upal_qspi_enter_memory_mapped(upal_qspi_t* q);
uetd_status_t upal_qspi_exit_memory_mapped(upal_qspi_t* q);

uetd_status_t upal_qspi_read(upal_qspi_t* q, uetd_u32_t addr, uetd_u8_t* buf, uetd_u32_t len, uetd_u32_t timeout_ms);
uetd_status_t upal_qspi_erase_sector(upal_qspi_t* q, uetd_u32_t addr, uetd_u32_t timeout_ms);
uetd_status_t upal_qspi_write_page(upal_qspi_t* q, uetd_u32_t addr, const uetd_u8_t* data, uetd_u32_t len, uetd_u32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_QSPI_H */
