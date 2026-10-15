/// @file include/ucore/status.h
/// @brief All UETD functions that can fail return uetd_status_t.

#ifndef UETD_STATUS_H
#define UETD_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UETD_OK = 0,
    UETD_BUSY = 1,
    UETD_ERROR = 2,
    UETD_TIMEOUT = 3,
    UETD_OVERFLOW = 4,
    UETD_UNDERFLOW = 5,
    UETD_INVALID_ARG = 6,
    UETD_NOT_SUPPORTED = 7,
    UETD_DMA_ERROR = 8,
    UETD_CACHE_ERROR = 9, /* buffer alignment violation (M7) */
    UETD_CRC_ERROR = 10,
    UETD_NOT_READY = 11,
    UETD_CONFLICT = 12, /* resource already in use */
} uetd_status_t;

const char* uetd_status_str(uetd_status_t s);

#ifdef __cplusplus
}
#endif

#endif /* UETD_STATUS_H */
