/// @file src/ucore/status.c
/// @brief Stable printable names for UEF result codes.

#include "uef/ucore/uef_status.h"

const char* uef_status_str(uef_status_t status) {
    switch (status) {
        case UEF_OK:
            return "ok";
        case UEF_BUSY:
            return "busy";
        case UEF_ERROR:
            return "error";
        case UEF_TIMEOUT:
            return "timeout";
        case UEF_OVERFLOW:
            return "overflow";
        case UEF_UNDERFLOW:
            return "underflow";
        case UEF_INVALID_ARG:
            return "invalid_argument";
        case UEF_NOT_SUPPORTED:
            return "not_supported";
        case UEF_DMA_ERROR:
            return "dma_error";
        case UEF_CACHE_ERROR:
            return "cache_error";
        case UEF_CRC_ERROR:
            return "crc_error";
        case UEF_NOT_READY:
            return "not_ready";
        case UEF_CONFLICT:
            return "conflict";
        default:
            /* Preserve a safe diagnostic string for unknown or future numeric status values. */
            return "unknown_status";
    }
}
