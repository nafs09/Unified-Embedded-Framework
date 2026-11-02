/// @file include/uproto/framing.h
/// @brief First-pass scaffold: Bounded optional COBS/SLIP framing profiles.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/CONNECTIVITY_SECURITY.md#framing-codecs-and-project-protocols.
#ifndef UETD_UPROTO_FRAMING_H
#define UETD_UPROTO_FRAMING_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "ucore/span.h"
#include "ucore/deadline.h"

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
typedef enum { UPROTO_FRAMING_COBS = 0, UPROTO_FRAMING_SLIP } uproto_framing_profile_t;
typedef struct { uproto_framing_profile_t profile; size_t maximum_payload, maximum_encoded; void* storage; size_t storage_bytes; uetd_u32_t partial_timeout_us; } uproto_framing_config_t;
typedef struct { void* storage; uetd_u64_t generation; bool initialized; } uproto_framing_t;

/// Validate exact escape/delimiter profile, maximum expansion, partial timeout and fixed storage.
uetd_status_t uproto_framing_init(uproto_framing_t* framing, const uproto_framing_config_t* config);
/// Check worst-case encoded extent and commit a complete frame without partial output on failure.
uetd_status_t uproto_framing_encode(uproto_framing_t* framing, ucore_read_span_t payload, ucore_write_span_t encoded, size_t* written);
/// Consume bounded input and preserve resynchronization work limits across corrupt/oversized/partial frames.
uetd_status_t uproto_framing_feed(uproto_framing_t* framing, ucore_read_span_t bytes, ucore_clock_stamp_t now, size_t* consumed);
/// Publish one validated complete payload; retain framing boundaries and reject undersized output.
uetd_status_t uproto_framing_next(uproto_framing_t* framing, ucore_write_span_t payload, size_t* written);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UPROTO_FRAMING_H */
