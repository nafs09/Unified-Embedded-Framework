/// @file include/uproto/network/future/mesh.h
/// @brief External-stack adapter contract for Thread and Zigbee networks.
///
/// Thread and Zigbee are separate profiles over IEEE 802.15.4. Providers may
/// bind an external module over UPAL SPI/I2C/UART or an integrated radio SDK.
/// Thread application traffic uses the IP service boundary; Zigbee application
/// traffic uses the bounded endpoint/cluster command operations below. UETD
/// does not implement commissioning, key management, routing, or ZCL here.

#ifndef UETD_UPROTO_FUTURE_MESH_H
#define UETD_UPROTO_FUTURE_MESH_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_MESH_NAME_MAX 16u
#define UPROTO_MESH_EVENT_DATA_MAX 128u

typedef enum {
    UPROTO_MESH_PROFILE_THREAD = 0,
    UPROTO_MESH_PROFILE_ZIGBEE,
} uproto_mesh_profile_t;

typedef enum {
    UPROTO_MESH_ROLE_END_DEVICE = 0,
    UPROTO_MESH_ROLE_ROUTER,
    UPROTO_MESH_ROLE_COMMISSIONER,
    UPROTO_MESH_ROLE_COORDINATOR,
} uproto_mesh_role_t;

typedef enum {
    UPROTO_MESH_STATE_STOPPED = 0,
    UPROTO_MESH_STATE_SCANNING,
    UPROTO_MESH_STATE_COMMISSIONING,
    UPROTO_MESH_STATE_ATTACHED,
    UPROTO_MESH_STATE_ERROR,
} uproto_mesh_state_t;

typedef struct {
    uproto_mesh_profile_t profile;
    uproto_mesh_role_t role;
    uetd_u8_t channel; /* 11..26 for the common 2.4 GHz band; zero selects */
    uetd_u16_t pan_id; /* 0xffff selects */
    uetd_u8_t network_name[UPROTO_MESH_NAME_MAX];
    uetd_u8_t network_name_length;
    uetd_u8_t extended_pan_id[8];
    uetd_u16_t credential_handle; /* opaque secure-store/provider reference; zero selects provider policy */
    uetd_u8_t endpoint;
} uproto_mesh_config_t;

typedef enum {
    UPROTO_MESH_EVENT_STATE = 0,
    UPROTO_MESH_EVENT_DEVICE_JOINED,
    UPROTO_MESH_EVENT_DEVICE_LEFT,
    UPROTO_MESH_EVENT_ATTRIBUTE_REPORT,
    UPROTO_MESH_EVENT_ZCL_COMMAND,
    UPROTO_MESH_EVENT_ERROR,
} uproto_mesh_event_kind_t;

typedef struct {
    uproto_mesh_event_kind_t kind;
    uproto_mesh_profile_t profile;
    uetd_u64_t peer_id;
    uetd_u16_t cluster_id;
    uetd_u16_t attribute_or_command_id;
    uetd_u16_t data_length;
    uetd_u8_t data[UPROTO_MESH_EVENT_DATA_MAX];
    uetd_u8_t status;
} uproto_mesh_event_t;

typedef struct {
    uetd_status_t (*start)(void* context, const uproto_mesh_config_t* config, bool form_network);
    uetd_status_t (*permit_join)(void* context, uetd_u16_t duration_seconds);
    uetd_status_t (*remove_device)(void* context, uetd_u64_t peer_id);
    uetd_status_t (*send_zigbee_command)(void* context, uetd_u64_t peer_id, uetd_u8_t endpoint,
                                         uetd_u16_t cluster_id, uetd_u16_t command_id,
                                         const uetd_u8_t* payload, uetd_u16_t payload_length);
    uetd_status_t (*leave)(void* context);
    uetd_status_t (*get_state)(void* context, uproto_mesh_state_t* state);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
    uetd_status_t (*next_event)(void* context, uproto_mesh_event_t* event);
} uproto_mesh_ops_t;

typedef struct {
    const uproto_mesh_ops_t* ops;
    void* context;
    uproto_mesh_profile_t profile;
    bool bound;
} uproto_mesh_t;

uetd_status_t uproto_mesh_bind(uproto_mesh_t* mesh, uproto_mesh_profile_t profile, const uproto_mesh_ops_t* ops, void* context);
uetd_status_t uproto_mesh_start(uproto_mesh_t* mesh, const uproto_mesh_config_t* config, bool form_network);
uetd_status_t uproto_mesh_permit_join(uproto_mesh_t* mesh, uetd_u16_t duration_seconds);
uetd_status_t uproto_mesh_remove_device(uproto_mesh_t* mesh, uetd_u64_t peer_id);
uetd_status_t uproto_mesh_send_zigbee_command(uproto_mesh_t* mesh, uetd_u64_t peer_id, uetd_u8_t endpoint,
                                              uetd_u16_t cluster_id, uetd_u16_t command_id,
                                              const uetd_u8_t* payload, uetd_u16_t payload_length);
uetd_status_t uproto_mesh_leave(uproto_mesh_t* mesh);
uetd_status_t uproto_mesh_get_state(uproto_mesh_t* mesh, uproto_mesh_state_t* state);
uetd_status_t uproto_mesh_poll(uproto_mesh_t* mesh, uetd_u16_t work_budget);
uetd_status_t uproto_mesh_next_event(uproto_mesh_t* mesh, uproto_mesh_event_t* event);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_FUTURE_MESH_H */
