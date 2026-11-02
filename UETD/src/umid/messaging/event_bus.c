/// @file src/umid/messaging/event_bus.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "umid/messaging/event_bus.h"

uetd_status_t umid_event_bus_init(umid_event_bus_t* bus, const umid_event_config_t* config) {
    /* TODO: Validate static topic/schema/subscriber identities, FIFO/latest policy and bounded storage. */
    (void)bus;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_event_publish(umid_event_bus_t* bus, const umid_event_t* event, umid_event_delivery_result_t* result) {
    /* TODO: Copy bounded payload and report exact partial delivery/overflow; critical inhibit remains an independent path. */
    (void)bus;
    (void)event;
    (void)result;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_event_receive(umid_event_bus_t* bus, uetd_u32_t subscriber_id, ucore_write_span_t payload, umid_mailbox_metadata_t* metadata) {
    /* TODO: Preserve per-subscriber delivery semantics and exact metadata without retaining caller buffers. */
    (void)bus;
    (void)subscriber_id;
    (void)payload;
    (void)metadata;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t umid_event_bus_reset(umid_event_bus_t* bus) {
    /* TODO: Require quiescent producers/consumers and invalidate old handles/delivery generations. */
    (void)bus;
    return UETD_NOT_SUPPORTED;
}

