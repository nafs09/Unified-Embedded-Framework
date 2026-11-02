/// @file include/umid/messaging/event_bus.h
/// @brief First-pass scaffold: Bounded typed topic and subscriber delivery.
/// Draft contract, not a stable ABI. Every operation currently returns
/// UETD_NOT_SUPPORTED without reading or changing caller storage or invoking a provider.
/// See docs/design/complete-system/PRODUCT_SERVICES.md#buffer-pools-events-and-dataflow.
#ifndef UETD_UMID_EVENT_BUS_H
#define UETD_UMID_EVENT_BUS_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "umid/messaging/mailbox.h"

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
typedef enum { UMID_EVENT_FIFO = 0, UMID_EVENT_LATEST } umid_event_delivery_t;
typedef struct { uetd_u32_t topic_id, schema_id; uetd_u16_t schema_version; ucore_read_span_t payload; ucore_clock_stamp_t captured; } umid_event_t;
typedef struct { uetd_u32_t subscriber_id, topic_id; umid_event_delivery_t delivery; void* storage; size_t storage_bytes, maximum_payload; uetd_u16_t depth; } umid_event_subscription_t;
typedef struct { size_t accepted, rejected; uetd_u64_t dropped_total; } umid_event_delivery_result_t;
typedef struct { const umid_event_subscription_t* subscriptions; size_t subscription_count; void* synchronization_provider; } umid_event_config_t;
typedef struct { const void* configuration; uetd_u64_t generation; bool initialized; } umid_event_bus_t;

/// Validate static topic/schema/subscriber identities, FIFO/latest policy and bounded storage.
uetd_status_t umid_event_bus_init(umid_event_bus_t* bus, const umid_event_config_t* config);
/// Copy bounded payload and report exact partial delivery/overflow; critical inhibit remains an independent path.
uetd_status_t umid_event_publish(umid_event_bus_t* bus, const umid_event_t* event, umid_event_delivery_result_t* result);
/// Preserve per-subscriber delivery semantics and exact metadata without retaining caller buffers.
uetd_status_t umid_event_receive(umid_event_bus_t* bus, uetd_u32_t subscriber_id, ucore_write_span_t payload, umid_mailbox_metadata_t* metadata);
/// Require quiescent producers/consumers and invalidate old handles/delivery generations.
uetd_status_t umid_event_bus_reset(umid_event_bus_t* bus);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMID_EVENT_BUS_H */
