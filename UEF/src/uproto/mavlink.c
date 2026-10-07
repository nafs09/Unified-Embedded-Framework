/// @file src/uproto/mavlink.c
/// @brief Source scaffold for the V1.2 public contract in uef/uproto/uproto_mavlink.h.
///
/// Implementation intent: Bind a selected MAVLink dialect and generated library; do not
///   duplicate upstream protocol definitions here.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uproto/uproto_mavlink.h>

uef_status_t uproto_mavlink_init(
    uproto_mavlink_t* m,
    uproto_stream_t transport,
    uef_u8_t sys_id,
    uef_u8_t comp_id
) {
    /* TODO(UEF UPROTO MAVLink init):
     * Validate the stream callbacks and local system/component IDs, clear parser/buffer state,
     * and bind the selected external MAVLink dialect parser. Keep generated upstream message
     * definitions outside UEF and record the transport's ownership/lifetime contract.
     */
    (void)m;
    (void)transport;
    (void)sys_id;
    (void)comp_id;
    return UEF_NOT_SUPPORTED;
}

uef_status_t uproto_mavlink_send(
    uproto_mavlink_t* m,
    const uef_u8_t* packed_msg,
    uef_u32_t len
) {
    /* TODO(UEF UPROTO MAVLink send):
     * Validate the packed message pointer/length and configured transport, then write the
     * complete already-packed frame with bounded partial-write handling. Preserve the supplied
     * upstream framing/checksum and distinguish transport failure from invalid input.
     */
    (void)m;
    (void)packed_msg;
    (void)len;
    return UEF_NOT_SUPPORTED;
}

bool uproto_mavlink_receive(
    uproto_mavlink_t* m,
    uef_u8_t* msg_buf,
    uef_u32_t* len
) {
    /* TODO(UEF UPROTO MAVLink receive):
     * Validate all pointers and output capacity, consume stream bytes incrementally through
     * the selected upstream parser, and copy a message only after a complete valid frame is
     * available. Return false for incomplete input without losing parser state; never overrun
     * `msg_buf`; record malformed/checksum failures through a diagnostic/counter extension if
     * required, since the current boolean return cannot distinguish malformed from incomplete.
     */
    (void)m;
    (void)msg_buf;
    (void)len;
    return false;
}
