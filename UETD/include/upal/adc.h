/// @file include/upal/adc.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_ADC_H
#define UPAL_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "upal/dma.h"
#include <uhal/target.h>

typedef struct {
    uetd_u8_t channel;
    uetd_u8_t rank; /* conversion sequence position */
    uetd_u8_t sampling_cycles;
    bool differential; /* true = differential pair */
} upal_adc_channel_cfg_t;

typedef struct {
    void* instance;
    upal_dma_t* dma;
    IRQn_Type irqn;
    uetd_u32_t dma_request;
    uetd_u32_t ext_trigger; /* timer trigger source */
    uetd_u32_t ext_trigger_edge;
} upal_adc_hw_t;

typedef struct {
    const upal_adc_hw_t* hw;
    const upal_adc_channel_cfg_t* channels;
    uetd_u8_t n_channels;
    uetd_u16_t* dma_buf; /* length = n_channels * n_avg */
    uetd_u32_t dma_buf_len;
    bool circular;
    upal_dma_callback_t callback;
    void* callback_ctx;
    volatile uetd_u32_t state;
} upal_adc_t;

uetd_status_t upal_adc_init(upal_adc_t* a, const upal_adc_hw_t* hw, const upal_adc_channel_cfg_t* channels, uetd_u8_t n_channels,
                           uetd_u16_t* dma_buf, uetd_u32_t dma_buf_len, bool circular, upal_dma_callback_t cb, void* ctx);

/* Calibrate — call before first use, after init, before start */
uetd_status_t upal_adc_calibrate(upal_adc_t* a);

uetd_status_t upal_adc_start(upal_adc_t* a);
void upal_adc_stop(upal_adc_t* a);

/* Blocking single scan — init and calibration sequences only */
uetd_status_t upal_adc_read_blocking(upal_adc_t* a, uetd_u16_t* results, uetd_u32_t timeout_ms);

void upal_adc_irq_handler(upal_adc_t* a);
void upal_adc_dma_handler(upal_adc_t* a);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_ADC_H */
