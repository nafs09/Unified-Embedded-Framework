/// @file src/uproto/can/canopen.c
/// @brief Fail-closed CANopen operations pending a selected device profile.
#include "uproto/can/canopen.h"

uetd_status_t uproto_canopen_validate(const uproto_canopen_config_t* config) {
    /* TODO: validate profile, dictionary, node, PDO/SDO, heartbeat and timing bounds. */
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_init(uproto_canopen_t* protocol, const uproto_canopen_config_t* config) {
    /* TODO: initialize caller-owned node, heartbeat, SDO and event storage. */
    (void)protocol;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_process_frame(uproto_canopen_t* protocol, uetd_u32_t can_id, const uetd_u8_t* data,
                                          uetd_u8_t length, uetd_u64_t received_at_us) {
    /* TODO: decode selected CANopen COB-IDs and queue bounded semantic events. */
    (void)protocol;
    (void)can_id;
    (void)data;
    (void)length;
    (void)received_at_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_next_event(uproto_canopen_t* protocol, uproto_canopen_event_t* event) {
    /* TODO: return one complete event and report queue loss explicitly. */
    (void)protocol;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_poll(uproto_canopen_t* protocol, uetd_u64_t now_us) {
    /* TODO: expire heartbeat/SDO work and advance bounded state transitions. */
    (void)protocol;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_sdo_read(uproto_canopen_t* protocol, uetd_u8_t node_id, uetd_u16_t index, uetd_u8_t subindex) {
    /* TODO: allocate one bounded SDO transaction and match its eventual response. */
    (void)protocol;
    (void)node_id;
    (void)index;
    (void)subindex;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_sdo_write(uproto_canopen_t* protocol, uetd_u8_t node_id, uetd_u16_t index, uetd_u8_t subindex,
                                      const uetd_u8_t* data, uetd_u16_t length) {
    /* TODO: enforce access rights/size and use the selected segmented transfer policy. */
    (void)protocol;
    (void)node_id;
    (void)index;
    (void)subindex;
    (void)data;
    (void)length;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_reset(uproto_canopen_t* protocol) {
    /* TODO: reset runtime peer/transaction state while preserving the profile. */
    (void)protocol;
    return UETD_NOT_SUPPORTED;
}
