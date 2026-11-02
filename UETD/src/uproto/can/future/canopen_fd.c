/// @file src/uproto/can/future/canopen_fd.c
/// @brief Fail-closed CANopen FD outline; no CiA 1301 wire implementation yet.
#include "uproto/can/future/canopen_fd.h"

uetd_status_t uproto_canopen_fd_validate(const uproto_canopen_fd_config_t* config) {
    /* TODO: select CiA 1301 revision/profile, FD provider and storage budgets. */
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_init(uproto_canopen_fd_t* protocol, const uproto_canopen_fd_config_t* config) {
    /* TODO: bind provider TX/RX, dictionary and bounded NMT/USDO/PDO state. */
    (void)protocol;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_process_frame(uproto_canopen_fd_t* protocol, const upal_fdcan_fd_frame_t* frame,
                                             uetd_u64_t received_at_us) {
    /* TODO: validate FD frame/DLC, decode standard services and queue events. */
    (void)protocol;
    (void)frame;
    (void)received_at_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_next_event(uproto_canopen_fd_t* protocol, uproto_canopen_fd_event_t* event) {
    /* TODO: return complete bounded events and report loss/quality changes. */
    (void)protocol;
    (void)event;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_poll(uproto_canopen_fd_t* protocol, uetd_u64_t now_us) {
    /* TODO: bounded NMT, heartbeat and USDO session/timeout processing. */
    (void)protocol;
    (void)now_us;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_usdo_read(uproto_canopen_fd_t* protocol, uetd_u8_t node_id, uetd_u16_t index,
                                         uetd_u8_t subindex) {
    /* TODO: standard USDO transfer/session matching within selected profile. */
    (void)protocol;
    (void)node_id;
    (void)index;
    (void)subindex;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_usdo_write(uproto_canopen_fd_t* protocol, uetd_u8_t node_id, uetd_u16_t index,
                                          uetd_u8_t subindex, const uetd_u8_t* data, uetd_u16_t length) {
    /* TODO: standard USDO encoding, rights/size checks and atomic OD commit. */
    (void)protocol;
    (void)node_id;
    (void)index;
    (void)subindex;
    (void)data;
    (void)length;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t uproto_canopen_fd_reset(uproto_canopen_fd_t* protocol) {
    /* TODO: reset transport transactions without weakening application policy. */
    (void)protocol;
    return UETD_NOT_SUPPORTED;
}
