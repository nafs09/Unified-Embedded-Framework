/// @file include/uproto/can/future/j1939.h
/// @brief Fail-closed operation outline for the planned J1939 protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_J1939_H
#define UPROTO_FUTURE_J1939_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uproto/scaffold_call.h"

/// Initialize the J1939 protocol state.
uetd_status_t uproto_j1939_init(uproto_future_call_t* call);
/// Process a received J1939 frame and update protocol state.
uetd_status_t uproto_j1939_process_frame(uproto_future_call_t* call);
/// Request a J1939 source address on the CAN bus.
uetd_status_t uproto_j1939_claim_address(uproto_future_call_t* call);
/// Send a J1939 parameter-group message.
uetd_status_t uproto_j1939_send_pgn(uproto_future_call_t* call);
/// Read a received J1939 parameter-group message.
uetd_status_t uproto_j1939_receive_pgn(uproto_future_call_t* call);
/// Reset the J1939 protocol state.
uetd_status_t uproto_j1939_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_J1939_H */
