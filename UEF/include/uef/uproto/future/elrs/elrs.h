/// @file include/uef/uproto/future/elrs/elrs.h
/// @brief Fail-closed operation outline for the planned ELRS protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_ELRS_H
#define UPROTO_FUTURE_ELRS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/uproto/future/scaffold_call.h"

uef_status_t uproto_elrs_validate_profile(uproto_future_call_t* call);
uef_status_t uproto_elrs_init(uproto_future_call_t* call);
uef_status_t uproto_elrs_process(uproto_future_call_t* call);
uef_status_t uproto_elrs_get_link_state(uproto_future_call_t* call);
uef_status_t uproto_elrs_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_ELRS_H */
