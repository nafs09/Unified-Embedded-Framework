/// @file src/uproto/uavcan.c
/// @brief Source scaffold for the V1.2 public contract in uproto/uavcan.h.
///
/// Implementation intent: Bind the selected DroneCAN/UAVCAN v0 transport and define bounded
///   transfer-ID and payload handling.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UETD_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uproto/uavcan.h>

uetd_status_t uproto_uavcan_init(uproto_uavcan_t* u, upal_can_t* can, uetd_u8_t node_id) {
    /* TODO(UETD UPROTO UAVCAN init):
     * Validate the CAN handle and node-ID range, initialize the selected upstream stack
     * instance from caller-owned or statically allocated storage, and bind bounded RX/TX
     * callbacks. Specify stack-version, allocator, transfer-ID, and instance lifetime policy.
     */
    (void)u;
    (void)can;
    (void)node_id;
    return UETD_NOT_SUPPORTED;
}

void uproto_uavcan_spin(uproto_uavcan_t* u) {
    /* TODO(UETD UPROTO UAVCAN spin):
     * Poll the upstream stack with a bounded work/time budget, feed queued CAN frames to its
     * parser, dispatch completed transfers, and submit pending TX frames without blocking an
     * interrupt. Preserve transfer state across calls and surface stack/transport errors.
     */
    (void)u;
}

uetd_status_t uproto_uavcan_broadcast(uproto_uavcan_t* u, const uetd_u8_t* payload, uetd_u32_t len, uetd_u16_t data_type_id,
                                     uetd_u8_t transfer_priority) {
    /* TODO(UETD UPROTO UAVCAN broadcast):
     * Validate payload, length, data-type ID, and priority against the selected transport and
     * stack limits. Allocate/advance the correct transfer ID, let the upstream stack segment
     * and encode the transfer, then report queue/transport failure accurately.
     */
    (void)u;
    (void)payload;
    (void)len;
    (void)data_type_id;
    (void)transfer_priority;
    return UETD_NOT_SUPPORTED;
}
