/// @file include/ucore/codec.h
/// @brief First-pass scaffold: Bounded binary cursors and wire values.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/CONNECTIVITY_SECURITY.md#framing-codecs-and-project-protocols.
#ifndef UETD_UCORE_CODEC_H
#define UETD_UCORE_CODEC_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Intended implementation: serialized task/main owner; no hidden heap.
 * Configurations, provider context and backing storage outlive the object.
 * Explicitly asynchronous operations retain buffers only after admission and
 * until terminal completion; synchronous spans are borrowed only for the call.
 * Schema IDs bind typed, versioned layouts/units; they do not permit arbitrary
 * native structs as a wire representation. Output/state commits and callback
 * context must be reviewed against the linked design before implementation.
 */
typedef enum { UCORE_CODEC_LITTLE_ENDIAN = 0, UCORE_CODEC_BIG_ENDIAN } ucore_codec_order_t;
typedef struct { ucore_read_span_t bytes; size_t offset; } ucore_codec_reader_t;
typedef struct { ucore_write_span_t bytes; size_t offset; } ucore_codec_writer_t;

/// Check remaining length, endian and offset arithmetic; commit cursor/value atomically.
uetd_status_t ucore_codec_read_u32(ucore_codec_reader_t* reader, ucore_codec_order_t order, uetd_u32_t* value);
/// Validate full extent before writing; preserve destination and offset on failure.
uetd_status_t ucore_codec_write_u32(ucore_codec_writer_t* writer, ucore_codec_order_t order, uetd_u32_t value);
/// Validate source and destination extents and overlap before committing bytes and cursor.
uetd_status_t ucore_codec_read_bytes(ucore_codec_reader_t* reader, ucore_write_span_t destination);
/// Encode explicit bytes without using native struct layout or advancing a failed cursor.
uetd_status_t ucore_codec_write_bytes(ucore_codec_writer_t* writer, ucore_read_span_t source);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UCORE_CODEC_H */
