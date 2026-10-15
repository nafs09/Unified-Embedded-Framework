/// @file include/uproto/mavlink.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPROTO_MAVLINK_H
#define UPROTO_MAVLINK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uproto/uproto.h"

/* Transport-independent MAVLink interface.
 * Actual message packing/unpacking uses the mavlink_c library headers
 * (included separately — not part of UETD). UETD provides the transport
 * binding and framing layer. */

typedef struct {
    uproto_stream_t transport;
    uetd_u8_t sys_id;
    uetd_u8_t comp_id;
    uetd_u8_t target_sys;
    uetd_u8_t target_comp;
    uetd_u32_t seq;
    uetd_u8_t rx_buf[280];
    uetd_u32_t rx_pos;
} uproto_mavlink_t;

uetd_status_t uproto_mavlink_init(uproto_mavlink_t* m, uproto_stream_t transport, uetd_u8_t sys_id, uetd_u8_t comp_id);
uetd_status_t uproto_mavlink_send(uproto_mavlink_t* m, const uetd_u8_t* packed_msg, uetd_u32_t len);
/* Returns true and fills msg_buf if a complete message was received */
bool uproto_mavlink_receive(uproto_mavlink_t* m, uetd_u8_t* msg_buf, uetd_u32_t* len);

#ifdef __cplusplus
}
#endif

#endif /* UPROTO_MAVLINK_H */
