/// @file src/uproto/uavcan.c
/// @brief Source scaffold for the V1.1 public contract in uef/uproto/uproto_uavcan.h.
///
/// Implementation intent: Bind the selected DroneCAN/UAVCAN v0 transport and define bounded
///   transfer-ID and payload handling.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uproto/uproto_uavcan.h>

uef_status_t uproto_uavcan_init(
    uproto_uavcan_t* u,
    upal_can_t* can,
    uef_u8_t node_id
) {
    /* TODO(UEF UPROTO UAVCAN init):
     * Validate the CAN handle and node-ID range, initialize the selected upstream stack
     * instance from caller-owned or statically allocated storage, and bind bounded RX/TX
     * callbacks. Specify stack-version, allocator, transfer-ID, and instance lifetime policy.
     */
    (void)u;
    (void)can;
    (void)node_id;
    return UEF_NOT_SUPPORTED;
}

void uproto_uavcan_spin(
    uproto_uavcan_t* u
) {
    /* TODO(UEF UPROTO UAVCAN spin):
     * Poll the upstream stack with a bounded work/time budget, feed queued CAN frames to its
     * parser, dispatch completed transfers, and submit pending TX frames without blocking an
     * interrupt. Preserve transfer state across calls and surface stack/transport errors.
     */
    (void)u;
}

uef_status_t uproto_uavcan_broadcast(
    uproto_uavcan_t* u,
    const uef_u8_t* payload,
    uef_u32_t len,
    uef_u16_t data_type_id,
    uef_u8_t transfer_priority
) {
    /* TODO(UEF UPROTO UAVCAN broadcast):
     * Validate payload, length, data-type ID, and priority against the selected transport and
     * stack limits. Allocate/advance the correct transfer ID, let the upstream stack segment
     * and encode the transfer, then report queue/transport failure accurately.
     */
    (void)u;
    (void)payload;
    (void)len;
    (void)data_type_id;
    (void)transfer_priority;
    return UEF_NOT_SUPPORTED;
}
