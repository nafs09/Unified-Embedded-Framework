/// @file include/uef/uproto/future/uavcan_v1/uavcan_v1.h
/// @brief Fail-closed operation outline for the planned UAVCAN_V1 protocol module.
///
/// The protocol profile and typed state/configuration are not complete. This
/// header is scaffold-only and must not be treated as a deployable API.
#ifndef UPROTO_FUTURE_UAVCAN_V1_H
#define UPROTO_FUTURE_UAVCAN_V1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/uproto/future/scaffold_call.h"

uef_status_t uproto_uavcan_v1_init(uproto_future_call_t* call);
uef_status_t uproto_uavcan_v1_spin(uproto_future_call_t* call);
uef_status_t uproto_uavcan_v1_publish(uproto_future_call_t* call);
uef_status_t uproto_uavcan_v1_subscribe(uproto_future_call_t* call);
uef_status_t uproto_uavcan_v1_reset(uproto_future_call_t* call);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_FUTURE_UAVCAN_V1_H */
