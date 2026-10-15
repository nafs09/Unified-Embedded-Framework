/// @file include/uproto/uproto.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPROTO_H
#define UPROTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

/* Generic byte-stream transport — implemented per physical transport */
typedef struct {
    uetd_status_t (*write)(const uetd_u8_t* data, uetd_u32_t len, void* ctx);
    uetd_u32_t (*read)(uetd_u8_t* buf, uetd_u32_t max_len, void* ctx);
    void* ctx;
} uproto_stream_t;

/* Waveform transport — for timer-DMA pulse protocols */
typedef struct {
    uetd_status_t (*output)(const uetd_u16_t* symbols, uetd_u32_t count, void* ctx);
    void* ctx;
} uproto_waveform_t;

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_H */
