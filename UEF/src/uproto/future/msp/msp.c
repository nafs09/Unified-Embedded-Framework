/// @file src/uproto/future/msp/msp.c
/// @brief Non-selectable, fail-closed MSP function outline.
#include "uef/uproto/future/msp/msp.h"

uef_status_t uproto_msp_validate_config(uproto_future_call_t* call) {
    /* TODO(UPROTO MSP VALIDATE_CONFIG): Check MSP v1/v2 selection, maximum frame sizes, version-specific checksum mode, and the caller-owned TX/RX storage bounds before accepting the profile.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_msp_init(uproto_future_call_t* call) {
    /* TODO(UPROTO MSP INIT): Initialize a bounded stream parser and command transaction state without allocating memory; do not publish a response until framing and checksum validation pass.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_msp_feed_bytes(uproto_future_call_t* call) {
    /* TODO(UPROTO MSP FEED_BYTES): Consume only the supplied byte span, resynchronize after malformed length/checksum data, and bound parser work by input_size.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_msp_encode_request(uproto_future_call_t* call) {
    /* TODO(UPROTO MSP ENCODE_REQUEST): Encode a selected MSP command with the correct version header, flags, length, command ID, checksum, and output-capacity checks.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_msp_take_response(uproto_future_call_t* call) {
    /* TODO(UPROTO MSP TAKE_RESPONSE): Copy one complete response only after matching its command/version and validating output capacity; preserve output_size and output bytes on incomplete input.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_msp_reset(uproto_future_call_t* call) {
    /* TODO(UPROTO MSP RESET): Discard partial frames and pending request state while preserving the selected profile and caller-owned storage.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
