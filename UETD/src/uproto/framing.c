/// @file src/uproto/framing.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "uproto/framing.h"

uetd_status_t uproto_framing_init(uproto_framing_t* framing, const uproto_framing_config_t* config) {
    /* TODO: Validate exact escape/delimiter profile, maximum expansion, partial timeout and fixed storage. */
    (void)framing;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_framing_encode(uproto_framing_t* framing, ucore_read_span_t payload, ucore_write_span_t encoded, size_t* written) {
    /* TODO: Check worst-case encoded extent and commit a complete frame without partial output on failure. */
    (void)framing;
    (void)payload;
    (void)encoded;
    (void)written;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_framing_feed(uproto_framing_t* framing, ucore_read_span_t bytes, ucore_clock_stamp_t now, size_t* consumed) {
    /* TODO: Consume bounded input and preserve resynchronization work limits across corrupt/oversized/partial frames. */
    (void)framing;
    (void)bytes;
    (void)now;
    (void)consumed;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_framing_next(uproto_framing_t* framing, ucore_write_span_t payload, size_t* written) {
    /* TODO: Publish one validated complete payload; retain framing boundaries and reject undersized output. */
    (void)framing;
    (void)payload;
    (void)written;
    return UETD_NOT_SUPPORTED;
}

