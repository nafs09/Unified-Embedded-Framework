/// @file src/uproto/can/future/medi_can.c
/// @brief Fail-closed Myovena MEDI CAN protocol outline, extending CANopen FD.
#include "uproto/can/future/medi_can.h"

uetd_status_t uproto_medi_can_validate(const uproto_medi_can_config_t* config) {
    /* TODO: review wire profile, bounded budgets, integrity and provider policy. */
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_init(uproto_medi_can_t* protocol, const uproto_medi_can_config_t* config) {
    /* TODO: bind base stack/clock/journal/validator/executor; reconcile recovery. */
    (void)protocol;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_submit(uproto_medi_can_t* protocol, const uproto_medi_can_command_t* command,
                                    uetd_u64_t now_us) {
    /* TODO: reserve bounded storage before admission, allocate identity/deadline. */
    (void)protocol;
    (void)command;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_process_frame(uproto_medi_can_t* protocol, const upal_fdcan_fd_frame_t* frame,
                                           uetd_u64_t received_at_us) {
    /* TODO: explicit extension dispatch, canonical decode, integrity and replay checks. */
    (void)protocol;
    (void)frame;
    (void)received_at_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_next_transmit(uproto_medi_can_t* protocol, upal_fdcan_fd_frame_t* frame, uetd_u64_t now_us) {
    /* TODO: bounded acknowledged delivery and status/result retransmission. */
    (void)protocol;
    (void)frame;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_poll(uproto_medi_can_t* protocol, uetd_u64_t now_us) {
    /* TODO: bound retry work; latch gaps, timeouts, resource faults and ambiguity. */
    (void)protocol;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_execute_ready(uproto_medi_can_t* protocol, uetd_u64_t now_us) {
    /* TODO: trusted non-bypassable gate with freshness/interlocks and durable dedup. */
    (void)protocol;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_record_result(uproto_medi_can_t* protocol, const uproto_medi_can_result_t* result,
                                           uetd_u64_t now_us) {
    /* TODO: verify trusted executor binding; retain/retransmit result and recovery evidence. */
    (void)protocol;
    (void)result;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_next_event(uproto_medi_can_t* protocol, uproto_medi_can_event_t* event) {
    /* TODO: bounded progress/fault events; overflow must latch execution inhibition. */
    (void)protocol;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_snapshot(const uproto_medi_can_t* protocol, uproto_medi_can_snapshot_t* snapshot) {
    /* TODO: publish coherent diagnostics without clearing latched faults/evidence. */
    (void)protocol;
    (void)snapshot;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_medi_can_reset(uproto_medi_can_t* protocol) {
    /* TODO: restart link/session only with replay retention and outcome reconciliation. */
    (void)protocol;
    return UETD_NOT_SUPPORTED;
}
