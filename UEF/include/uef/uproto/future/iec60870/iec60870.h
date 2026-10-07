/// @file include/uef/uproto/future/iec60870/iec60870.h
/// @brief Fail-closed operation outline for the planned IEC60870 protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_IEC60870_H
#define UPROTO_FUTURE_IEC60870_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/uproto/future/scaffold_call.h"

uef_status_t uproto_iec60870_validate_profile(uproto_future_call_t* call);
uef_status_t uproto_iec60870_init(uproto_future_call_t* call);
uef_status_t uproto_iec60870_process(uproto_future_call_t* call);
uef_status_t uproto_iec60870_send_asdu(uproto_future_call_t* call);
uef_status_t uproto_iec60870_next_event(uproto_future_call_t* call);
uef_status_t uproto_iec60870_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_IEC60870_H */
