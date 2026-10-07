/// @file src/uproto/future/elrs/elrs.c
/// @brief Non-selectable, fail-closed ELRS function outline.
#include "uef/uproto/future/elrs/elrs.h"

uef_status_t uproto_elrs_validate_profile(uproto_future_call_t* call) {
    /* TODO(UPROTO ELRS VALIDATE_PROFILE): Decide which ExpressLRS link fields are carried by the CRSF API and validate any receiver-specific settings without duplicating CRSF framing.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_elrs_init(uproto_future_call_t* call) {
    /* TODO(UPROTO ELRS INIT): Bind the chosen CRSF instance or receiver transport and initialize ELRS-specific link/failsafe state using caller-owned storage.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_elrs_process(uproto_future_call_t* call) {
    /* TODO(UPROTO ELRS PROCESS): Update ELRS-specific link data only from validated CRSF frames; define how receiver metadata freshness and reconnect transitions are represented.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_elrs_get_link_state(uproto_future_call_t* call) {
    /* TODO(UPROTO ELRS GET_LINK_STATE): Return a coherent snapshot of link quality, RSSI/SNR, RF mode, packet rate, and failsafe state after the specification selects the exact fields and units.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
uef_status_t uproto_elrs_reset(uproto_future_call_t* call) {
    /* TODO(UPROTO ELRS RESET): Clear link freshness and receiver session state without resetting a shared CRSF parser owned by another component.
     * Replace the type-erased call envelope with typed fixed-size arguments,
     * define status/output-preservation semantics, and add the corresponding
     * UEF-owned instance template only after contract review.
     */
    return uproto_future_scaffold_result(call);
}
