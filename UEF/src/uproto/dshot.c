/// @file src/uproto/dshot.c
/// @brief Source scaffold for the V1.2 public contract in uef/uproto/uproto_dshot.h.
///
/// Implementation intent: Encode the 16-bit DSHOT frame and telemetry bit, then hand symbols to
///   timer or UART transport with DMA-safe ownership.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uproto/uproto_dshot.h>

uef_status_t uproto_dshot_init(
    uproto_dshot_t* d
) {
    /* TODO(UEF UPROTO DSHOT init):
     * Validate rate and selected timer/UART union member, configure the output transport, and
     * initialize the symbol buffer. Reject impossible transport/rate combinations before any
     * pin or DMA state is changed.
     */
    (void)d;
    return UEF_NOT_SUPPORTED;
}

uef_status_t uproto_dshot_send(
    uproto_dshot_t* d,
    uef_u16_t throttle,
    bool telemetry
) {
    /* TODO(UEF UPROTO DSHOT send):
     * Validate throttle in the declared range and reject overlapping DMA transfers; encode
     * the 11-bit value, telemetry request bit, checksum, and 16 data plus 2 reset symbols,
     * then start the selected transport without waiting for the waveform to finish.
     */
    (void)d;
    (void)throttle;
    (void)telemetry;
    return UEF_NOT_SUPPORTED;
}

uef_status_t uproto_dshot_command(
    uproto_dshot_t* d,
    uef_u8_t command
) {
    /* TODO(UEF UPROTO DSHOT command):
     * Validate the command against the selected DSHOT command table, encode it using the
     * required command/telemetry framing, and submit it through the same nonblocking transport
     * ownership path as throttle frames. Do not silently clamp unknown command values.
     */
    (void)d;
    (void)command;
    return UEF_NOT_SUPPORTED;
}
