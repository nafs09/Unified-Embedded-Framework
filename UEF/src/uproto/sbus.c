/// @file src/uproto/sbus.c
/// @brief Source scaffold for the V1.2 public contract in uef/uproto/uproto_sbus.h.
///
/// Implementation intent: Decode the fixed SBUS frame, validate framing flags, and publish
///   channels only after a complete frame.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uproto/uproto_sbus.h>

uef_status_t uproto_sbus_init(
    uproto_sbus_t* s,
    upal_uart_t* uart
) {
    /* TODO(UEF UPROTO SBUS init):
     * Validate the decoder and UART, clear partial-frame state, configure inverted 100 kbaud
     * 8E2 transport through UPAL, and register the bounded receive path. Do not expose an old
     * or zero-filled frame as a newly received frame.
     */
    (void)s;
    (void)uart;
    return UEF_NOT_SUPPORTED;
}

void uproto_sbus_process(
    uproto_sbus_t* s
) {
    /* TODO(UEF UPROTO SBUS process):
     * Consume only the bytes already available from the UART/ring buffer, resynchronize on
     * framing loss, require the full 25-byte frame, unpack the 16 channels, validate footer
     * and flags, then atomically publish the frame and timestamp. Bound work per call and
     * leave the last valid frame unchanged when input is malformed.
     */
    (void)s;
}

bool uproto_sbus_available(
    const uproto_sbus_t* s
) {
    /* TODO(UEF UPROTO SBUS availability):
     * Define whether availability means a valid frame exists or a frame arrived since the
     * previous get; implement that contract using an explicit valid/new generation marker so
     * callers never infer freshness from zero-valued channel data.
     */
    (void)s;
    return false;
}

void uproto_sbus_get(
    uproto_sbus_t* s,
    uproto_sbus_frame_t* out
) {
    /* TODO(UEF UPROTO SBUS get):
     * Validate both pointers, copy one coherent last-valid frame while respecting ISR/task
     * ownership, and define whether this call consumes the new-frame marker. Preserve the
     * previous caller output if no valid frame has ever arrived.
     */
    (void)s;
    (void)out;
}
