/// @file include/uef/uproto/uproto_uavcan.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPROTO_UAVCAN_H
#define UPROTO_UAVCAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_can.h"

/* UEF provides the CAN transport binding for UAVCAN/DroneCAN.
 * Protocol stack (libcanard or libuavcan) is a project dependency,
 * not bundled with UEF. This header provides the glue layer. */

typedef struct {
    upal_can_t*  can;
    uef_u8_t     node_id;
    uef_u64_t    tx_timestamp_us;
    /* libcanard CanardInstance goes here in the project implementation */
    void*        canard_instance;
} uproto_uavcan_t;

/* Transport adapter — maps UAVCAN stack callbacks to UPAL CAN */
uef_status_t uproto_uavcan_init(uproto_uavcan_t* u,
                                  upal_can_t* can, uef_u8_t node_id);
void         uproto_uavcan_spin(uproto_uavcan_t* u);   /* call periodically */
uef_status_t uproto_uavcan_broadcast(uproto_uavcan_t* u,
                                       const uef_u8_t* payload,
                                       uef_u32_t len,
                                       uef_u16_t data_type_id,
                                       uef_u8_t transfer_priority);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_UAVCAN_H */
