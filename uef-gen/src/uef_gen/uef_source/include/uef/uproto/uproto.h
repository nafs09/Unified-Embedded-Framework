/// @file include/uef/uproto/uproto.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPROTO_H
#define UPROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

/* Generic byte-stream transport — implemented per physical transport */
typedef struct {
    uef_status_t (*write)(const uef_u8_t* data, uef_u32_t len, void* ctx);
    uef_u32_t    (*read)(uef_u8_t* buf, uef_u32_t max_len, void* ctx);
    void*         ctx;
} uproto_stream_t;

/* Waveform transport — for timer-DMA pulse protocols */
typedef struct {
    uef_status_t (*output)(const uef_u16_t* symbols, uef_u32_t count,
                            void* ctx);
    void*         ctx;
} uproto_waveform_t;

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_H */
