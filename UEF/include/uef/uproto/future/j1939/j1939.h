/// @file include/uef/uproto/future/j1939/j1939.h
/// @brief Fail-closed operation outline for the planned J1939 protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_J1939_H
#define UPROTO_FUTURE_J1939_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/uproto/future/scaffold_call.h"

uef_status_t uproto_j1939_init(uproto_future_call_t* call);
uef_status_t uproto_j1939_process_frame(uproto_future_call_t* call);
uef_status_t uproto_j1939_claim_address(uproto_future_call_t* call);
uef_status_t uproto_j1939_send_pgn(uproto_future_call_t* call);
uef_status_t uproto_j1939_receive_pgn(uproto_future_call_t* call);
uef_status_t uproto_j1939_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_J1939_H */
