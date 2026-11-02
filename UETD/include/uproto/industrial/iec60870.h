/// @file include/uproto/industrial/iec60870.h
/// @brief Fail-closed operation outline for the planned IEC60870 protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_IEC60870_H
#define UPROTO_FUTURE_IEC60870_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uproto/scaffold_call.h"

/// Validate the IEC 60870 profile and its resource requirements.
uetd_status_t uproto_iec60870_validate_profile(uproto_future_call_t* call);
/// Initialize the IEC 60870 protocol state.
uetd_status_t uproto_iec60870_init(uproto_future_call_t* call);
/// Process received IEC 60870 data and update protocol state.
uetd_status_t uproto_iec60870_process(uproto_future_call_t* call);
/// Encode and send an IEC 60870 application service data unit.
uetd_status_t uproto_iec60870_send_asdu(uproto_future_call_t* call);
/// Return the next decoded IEC 60870 event, if one is queued.
uetd_status_t uproto_iec60870_next_event(uproto_future_call_t* call);
/// Reset the IEC 60870 protocol state.
uetd_status_t uproto_iec60870_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_IEC60870_H */
