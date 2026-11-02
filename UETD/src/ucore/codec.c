/// @file src/ucore/codec.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "ucore/codec.h"

uetd_status_t ucore_codec_read_u32(ucore_codec_reader_t* reader, ucore_codec_order_t order, uetd_u32_t* value) {
    /* TODO: Check remaining length, endian and offset arithmetic; commit cursor/value atomically. */
    (void)reader;
    (void)order;
    (void)value;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t ucore_codec_write_u32(ucore_codec_writer_t* writer, ucore_codec_order_t order, uetd_u32_t value) {
    /* TODO: Validate full extent before writing; preserve destination and offset on failure. */
    (void)writer;
    (void)order;
    (void)value;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t ucore_codec_read_bytes(ucore_codec_reader_t* reader, ucore_write_span_t destination) {
    /* TODO: Validate source and destination extents and overlap before committing bytes and cursor. */
    (void)reader;
    (void)destination;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t ucore_codec_write_bytes(ucore_codec_writer_t* writer, ucore_read_span_t source) {
    /* TODO: Encode explicit bytes without using native struct layout or advancing a failed cursor. */
    (void)writer;
    (void)source;
    return UETD_NOT_SUPPORTED;
}

