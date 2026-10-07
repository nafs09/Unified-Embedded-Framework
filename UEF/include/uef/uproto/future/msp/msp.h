/// @file include/uef/uproto/future/msp/msp.h
/// @brief Fail-closed operation outline for the planned MSP protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_MSP_H
#define UPROTO_FUTURE_MSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/uproto/future/scaffold_call.h"

uef_status_t uproto_msp_validate_config(uproto_future_call_t* call);
uef_status_t uproto_msp_init(uproto_future_call_t* call);
uef_status_t uproto_msp_feed_bytes(uproto_future_call_t* call);
uef_status_t uproto_msp_encode_request(uproto_future_call_t* call);
uef_status_t uproto_msp_take_response(uproto_future_call_t* call);
uef_status_t uproto_msp_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_MSP_H */
