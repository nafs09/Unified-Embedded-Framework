/// @file include/uef/ucore/uef_status.h
/// @brief All UEF functions that can fail return uef_status_t.

#ifndef UEF_STATUS_H
#define UEF_STATUS_H


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UEF_OK              = 0,
    UEF_BUSY            = 1,
    UEF_ERROR           = 2,
    UEF_TIMEOUT         = 3,
    UEF_OVERFLOW        = 4,
    UEF_UNDERFLOW       = 5,
    UEF_INVALID_ARG     = 6,
    UEF_NOT_SUPPORTED   = 7,
    UEF_DMA_ERROR       = 8,
    UEF_CACHE_ERROR     = 9,    /* buffer alignment violation (M7) */
    UEF_CRC_ERROR       = 10,
    UEF_NOT_READY       = 11,
    UEF_CONFLICT        = 12,   /* resource already in use */
} uef_status_t;

const char* uef_status_str(uef_status_t s);

#ifdef __cplusplus
}
#endif

#endif /* UEF_STATUS_H */
