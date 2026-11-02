/// @file include/ucore/span.h
/// @brief Checked byte spans and size arithmetic; errors preserve outputs.
/// Copy permits overlap (memmove semantics); empty spans may have NULL data.
/// See docs/design/complete-system/FOUNDATIONS.md#core-values-errors-and-checked-storage.
#ifndef UETD_UCORE_SPAN_H
#define UETD_UCORE_SPAN_H

#include "ucore/status.h"
#include "ucore/types.h"

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
typedef struct { const uetd_u8_t* data; size_t size; } ucore_read_span_t;
typedef struct { uetd_u8_t* data; size_t size; } ucore_write_span_t;

/// Check addition against SIZE_MAX before committing the result.
uetd_status_t ucore_size_add(size_t left, size_t right, size_t* result);
/// Check multiplication, including zero operands, without intermediate overflow.
uetd_status_t ucore_size_multiply(size_t left, size_t right, size_t* result);
/// Check offset/length and address overflow; preserve the source lifetime and result on failure.
uetd_status_t ucore_span_slice(ucore_read_span_t source, size_t offset, size_t length, ucore_read_span_t* result);
/// Validate both extents and the documented overlap policy before copying any byte.
uetd_status_t ucore_span_copy(ucore_write_span_t destination, ucore_read_span_t source);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UCORE_SPAN_H */
