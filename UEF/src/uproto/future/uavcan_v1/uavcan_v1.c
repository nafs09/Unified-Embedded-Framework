/// @file src/uproto/future/uavcan_v1/uavcan_v1.c
/// @brief Non-selectable, fail-closed UAVCAN_V1 function outline.
#include "uef/uproto/future/uavcan_v1/uavcan_v1.h"

uef_status_t uproto_uavcan_v1_init(uproto_future_call_t* call) {
    /* TODO(UPROTO UAVCAN_V1 INIT): Bind the selected UAVCAN v1/Cyphal stack to the FDCAN-FD adapter, validate node identity, and install bounded static-memory callbacks.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_uavcan_v1_spin(uproto_future_call_t* call) {
    /* TODO(UPROTO UAVCAN_V1 SPIN): Run a bounded stack service step that drains a limited number of CAN-FD frames, dispatches completed transfers, and queues eligible responses.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_uavcan_v1_publish(uproto_future_call_t* call) {
    /* TODO(UPROTO UAVCAN_V1 PUBLISH): Validate the selected DSDL subject/message and payload bounds, obtain the correct transfer-ID state, and enqueue without blocking.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_uavcan_v1_subscribe(uproto_future_call_t* call) {
    /* TODO(UPROTO UAVCAN_V1 SUBSCRIBE): Register a statically bounded subscription and dispatch only checksum-valid, correctly typed transfers to the supplied handler.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_uavcan_v1_reset(uproto_future_call_t* call) {
    /* TODO(UPROTO UAVCAN_V1 RESET): Clear transfer/session state while preserving configured node identity, stack storage, and registered subscriptions as specified.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
