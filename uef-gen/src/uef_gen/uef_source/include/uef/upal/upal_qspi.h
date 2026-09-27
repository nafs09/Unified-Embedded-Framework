/// @file include/uef/upal/upal_qspi.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_QSPI_H
#define UPAL_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_dma.h"
#include "uef/uhal/uhal_gpio.h"

typedef struct {
    void*            instance;
    upal_dma_t*      dma;
    uef_u32_t        dma_request;
    uhal_gpio_pin_t  clk_pin, ncs_pin;
    uhal_gpio_pin_t  io0_pin, io1_pin, io2_pin, io3_pin;
    uef_u8_t         af;
    uef_u32_t        flash_size_bytes;
    uef_u32_t        max_clock_hz;
} upal_qspi_hw_t;

typedef struct {
    const upal_qspi_hw_t* hw;
    volatile uef_u32_t    state;
} upal_qspi_t;

uef_status_t upal_qspi_init(upal_qspi_t* q, const upal_qspi_hw_t* hw);

/* Memory-mapped mode: flash at QUADSPI base address */
uef_status_t upal_qspi_enter_memory_mapped(upal_qspi_t* q);
uef_status_t upal_qspi_exit_memory_mapped(upal_qspi_t* q);

uef_status_t upal_qspi_read(upal_qspi_t* q, uef_u32_t addr,
                              uef_u8_t* buf, uef_u32_t len,
                              uef_u32_t timeout_ms);
uef_status_t upal_qspi_erase_sector(upal_qspi_t* q, uef_u32_t addr,
                                      uef_u32_t timeout_ms);
uef_status_t upal_qspi_write_page(upal_qspi_t* q, uef_u32_t addr,
                                    const uef_u8_t* data, uef_u32_t len,
                                    uef_u32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_QSPI_H */
