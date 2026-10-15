/// @file src/ucore/status.c
/// @brief Stable printable names for UETD result codes.

#include "ucore/status.h"

const char* uetd_status_str(uetd_status_t status) {
    switch (status) {
    case UETD_OK: return "ok";
    case UETD_BUSY: return "busy";
    case UETD_ERROR: return "error";
    case UETD_TIMEOUT: return "timeout";
    case UETD_OVERFLOW: return "overflow";
    case UETD_UNDERFLOW: return "underflow";
    case UETD_INVALID_ARG: return "invalid_argument";
    case UETD_NOT_SUPPORTED: return "not_supported";
    case UETD_DMA_ERROR: return "dma_error";
    case UETD_CACHE_ERROR: return "cache_error";
    case UETD_CRC_ERROR: return "crc_error";
    case UETD_NOT_READY: return "not_ready";
    case UETD_CONFLICT: return "conflict";
    default:
        /* Preserve a safe diagnostic string for unknown or future numeric status values. */
        return "unknown_status";
    }
}
