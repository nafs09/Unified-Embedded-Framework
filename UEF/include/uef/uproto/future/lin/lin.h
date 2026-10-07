/// @file include/uef/uproto/future/lin/lin.h
/// @brief Fail-closed operation outline for the planned LIN protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_LIN_H
#define UPROTO_FUTURE_LIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/uproto/future/scaffold_call.h"

uef_status_t uproto_lin_validate_schedule(uproto_future_call_t* call);
uef_status_t uproto_lin_init(uproto_future_call_t* call);
uef_status_t uproto_lin_schedule_header(uproto_future_call_t* call);
uef_status_t uproto_lin_process_response(uproto_future_call_t* call);
uef_status_t uproto_lin_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_LIN_H */
