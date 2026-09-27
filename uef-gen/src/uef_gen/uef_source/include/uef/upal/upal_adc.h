/// @file include/uef/upal/upal_adc.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_ADC_H
#define UPAL_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_dma.h"
#include <uef/uhal/target.h>

typedef struct {
    uef_u8_t channel;
    uef_u8_t rank;            /* conversion sequence position */
    uef_u8_t sampling_cycles;
    bool     differential;   /* true = differential pair */
} upal_adc_channel_cfg_t;

typedef struct {
    void*         instance;
    upal_dma_t*   dma;
    IRQn_Type     irqn;
    uef_u32_t     dma_request;
    uef_u32_t     ext_trigger;        /* timer trigger source */
    uef_u32_t     ext_trigger_edge;
} upal_adc_hw_t;

typedef struct {
    const upal_adc_hw_t*          hw;
    const upal_adc_channel_cfg_t* channels;
    uef_u8_t                      n_channels;
    uef_u16_t*                    dma_buf;   /* length = n_channels * n_avg */
    uef_u32_t                     dma_buf_len;
    bool                          circular;
    upal_dma_callback_t           callback;
    void*                         callback_ctx;
    volatile uef_u32_t            state;
} upal_adc_t;

uef_status_t upal_adc_init(upal_adc_t* a, const upal_adc_hw_t* hw,
                             const upal_adc_channel_cfg_t* channels,
                             uef_u8_t n_channels,
                             uef_u16_t* dma_buf, uef_u32_t dma_buf_len,
                             bool circular,
                             upal_dma_callback_t cb, void* ctx);

/* Calibrate — call before first use, after init, before start */
uef_status_t upal_adc_calibrate(upal_adc_t* a);

uef_status_t upal_adc_start(upal_adc_t* a);
void         upal_adc_stop(upal_adc_t* a);

/* Blocking single scan — init and calibration sequences only */
uef_status_t upal_adc_read_blocking(upal_adc_t* a, uef_u16_t* results,
                                      uef_u32_t timeout_ms);

void upal_adc_irq_handler(upal_adc_t* a);
void upal_adc_dma_handler(upal_adc_t* a);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_ADC_H */
