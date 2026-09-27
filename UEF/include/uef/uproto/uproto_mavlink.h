/// @file include/uef/uproto/uproto_mavlink.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPROTO_MAVLINK_H
#define UPROTO_MAVLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/uproto/uproto.h"

/* Transport-independent MAVLink interface.
 * Actual message packing/unpacking uses the mavlink_c library headers
 * (included separately — not part of UEF). UEF provides the transport
 * binding and framing layer. */

typedef struct {
    uproto_stream_t  transport;
    uef_u8_t         sys_id;
    uef_u8_t         comp_id;
    uef_u8_t         target_sys;
    uef_u8_t         target_comp;
    uef_u32_t        seq;
    uef_u8_t         rx_buf[280];
    uef_u32_t        rx_pos;
} uproto_mavlink_t;

uef_status_t uproto_mavlink_init(uproto_mavlink_t* m,
                                   uproto_stream_t transport,
                                   uef_u8_t sys_id, uef_u8_t comp_id);
uef_status_t uproto_mavlink_send(uproto_mavlink_t* m,
                                   const uef_u8_t* packed_msg,
                                   uef_u32_t len);
/* Returns true and fills msg_buf if a complete message was received */
bool uproto_mavlink_receive(uproto_mavlink_t* m,
                              uef_u8_t* msg_buf, uef_u32_t* len);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_MAVLINK_H */
