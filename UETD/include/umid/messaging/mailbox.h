/// @file include/umid/messaging/mailbox.h
/// @brief First-pass scaffold: Coherent copied latest-value publication.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#buffer-pools-events-and-dataflow.
#ifndef UETD_UMID_MAILBOX_H
#define UETD_UMID_MAILBOX_H

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
typedef struct { ucore_clock_stamp_t captured; uetd_u64_t sequence; uetd_u32_t quality; } umid_mailbox_metadata_t;
typedef struct { void* storage; size_t storage_bytes, value_bytes; void* synchronization_provider; } umid_mailbox_config_t;
typedef struct { void* storage; uetd_u64_t generation; bool initialized; } umid_mailbox_t;

/// Validate fixed-size storage and a real synchronization route for the declared producer/consumer topology.
uetd_status_t umid_mailbox_init(umid_mailbox_t* mailbox, const umid_mailbox_config_t* config);
/// Commit the entire value and metadata coherently; detect sequence exhaustion/reset.
uetd_status_t umid_mailbox_publish(umid_mailbox_t* mailbox, ucore_read_span_t value, const umid_mailbox_metadata_t* metadata);
/// Read one coherent generation, preserving destination on no value or capacity failure.
uetd_status_t umid_mailbox_read(const umid_mailbox_t* mailbox, ucore_write_span_t value, umid_mailbox_metadata_t* metadata);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_MAILBOX_H */
