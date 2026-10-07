/// @file include/uef/upal/upal_dac.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UPAL_DAC_H
#define UPAL_DAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_dma.h"

typedef struct {
    void*       instance;
    upal_dma_t* dma_ch1;
    upal_dma_t* dma_ch2;
    uef_u32_t   dma_request_ch1;
    uef_u32_t   dma_request_ch2;
} upal_dac_hw_t;

typedef struct {
    const upal_dac_hw_t* hw;
    volatile uef_u32_t   state;
} upal_dac_t;

uef_status_t upal_dac_init(upal_dac_t* d, const upal_dac_hw_t* hw);

/* Immediate 12-bit output */
void upal_dac_write(upal_dac_t* d, uef_u8_t channel, uef_u16_t value_12bit);

/* DMA waveform — timer-triggered arbitrary analogue output */
uef_status_t upal_dac_dma_start(upal_dac_t* d, uef_u8_t channel,
                                  const uef_u16_t* samples, uef_u32_t count,
                                  bool circular,
                                  upal_dma_callback_t cb, void* ctx);
void upal_dac_dma_stop(upal_dac_t* d, uef_u8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_DAC_H */
