/// @file include/uproto/rc/future/elrs.h
/// @brief Fail-closed operation outline for the planned ELRS protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_ELRS_H
#define UPROTO_FUTURE_ELRS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uproto/scaffold_call.h"

/// Validate the ExpressLRS profile and its resource requirements.
uetd_status_t uproto_elrs_validate_profile(uproto_future_call_t* call);
/// Initialize the ExpressLRS adapter state.
uetd_status_t uproto_elrs_init(uproto_future_call_t* call);
/// Process received ExpressLRS data and update link state.
uetd_status_t uproto_elrs_process(uproto_future_call_t* call);
/// Return the latest ExpressLRS link-quality and connection state.
uetd_status_t uproto_elrs_get_link_state(uproto_future_call_t* call);
/// Reset the ExpressLRS adapter state.
uetd_status_t uproto_elrs_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_ELRS_H */
