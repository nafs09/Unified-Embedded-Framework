/// @file include/uproto/network/mqtt.h
/// @brief Bounded MQTT client adapter contract (3.1.1 or 5.0).
///
/// MQTT framing, keep-alive scheduling, acknowledgement tracking, reconnect
/// policy, and optional TLS are supplied by a selected provider. A provider
/// can use the UPROTO IP socket facade, a vendor SDK, or a coprocessor's
/// socket/protocol offload. No credentials or packet buffers are allocated by
/// this UETD facade.

#ifndef UETD_UPROTO_MQTT_H
#define UETD_UPROTO_MQTT_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "uproto/network/ip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_MQTT_TOPIC_MAX 512u
#define UPROTO_MQTT_EVENT_DATA_MAX 256u
#define UPROTO_MQTT_PACKET_MAX 1024u

typedef enum {
    UPROTO_MQTT_VERSION_3_1_1 = 4,
    UPROTO_MQTT_VERSION_5 = 5,
} uproto_mqtt_version_t;

typedef enum {
    UPROTO_MQTT_QOS_0 = 0,
    UPROTO_MQTT_QOS_1 = 1,
    UPROTO_MQTT_QOS_2 = 2,
} uproto_mqtt_qos_t;

typedef struct {
    uproto_mqtt_version_t version;
    uproto_ip_endpoint_t broker;
    const uetd_u8_t* client_id;
    uetd_u8_t client_id_length;
    uetd_u16_t keep_alive_seconds;
    uetd_u16_t maximum_packet_bytes;
    bool clean_session;
    bool use_tls;
    uetd_u16_t credential_handle; /* opaque provider/key-store reference; zero means none */
} uproto_mqtt_config_t;

typedef enum {
    UPROTO_MQTT_EVENT_CONNECTED = 0,
    UPROTO_MQTT_EVENT_DISCONNECTED,
    UPROTO_MQTT_EVENT_PUBLISHED,
    UPROTO_MQTT_EVENT_MESSAGE,
    UPROTO_MQTT_EVENT_ERROR,
} uproto_mqtt_event_kind_t;

typedef struct {
    uproto_mqtt_event_kind_t kind;
    uetd_u16_t packet_id;
    uetd_u16_t topic_length;
    uetd_u16_t payload_length;
    uetd_u8_t topic[UPROTO_MQTT_TOPIC_MAX];
    uetd_u8_t payload[UPROTO_MQTT_EVENT_DATA_MAX];
    uetd_u8_t reason_code;
} uproto_mqtt_event_t;

typedef struct {
    uetd_status_t (*connect)(void* context, const uproto_mqtt_config_t* config);
    uetd_status_t (*disconnect)(void* context);
    uetd_status_t (*publish)(void* context, const uetd_u8_t* topic, uetd_u16_t topic_length,
                             const uetd_u8_t* payload, uetd_u16_t payload_length, uproto_mqtt_qos_t qos, bool retain);
    uetd_status_t (*subscribe)(void* context, const uetd_u8_t* topic_filter, uetd_u16_t topic_length, uproto_mqtt_qos_t qos);
    uetd_status_t (*unsubscribe)(void* context, const uetd_u8_t* topic_filter, uetd_u16_t topic_length);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
    uetd_status_t (*next_event)(void* context, uproto_mqtt_event_t* event);
} uproto_mqtt_ops_t;

typedef struct {
    const uproto_mqtt_ops_t* ops;
    void* context;
    uetd_u16_t maximum_packet_bytes;
    bool bound;
} uproto_mqtt_t;

uetd_status_t uproto_mqtt_bind(uproto_mqtt_t* client, const uproto_mqtt_ops_t* ops, void* context);
uetd_status_t uproto_mqtt_connect(uproto_mqtt_t* client, const uproto_mqtt_config_t* config);
uetd_status_t uproto_mqtt_disconnect(uproto_mqtt_t* client);
uetd_status_t uproto_mqtt_publish(uproto_mqtt_t* client, const uetd_u8_t* topic, uetd_u16_t topic_length,
                                  const uetd_u8_t* payload, uetd_u16_t payload_length, uproto_mqtt_qos_t qos, bool retain);
uetd_status_t uproto_mqtt_subscribe(uproto_mqtt_t* client, const uetd_u8_t* topic_filter, uetd_u16_t topic_length,
                                    uproto_mqtt_qos_t qos);
uetd_status_t uproto_mqtt_unsubscribe(uproto_mqtt_t* client, const uetd_u8_t* topic_filter, uetd_u16_t topic_length);
uetd_status_t uproto_mqtt_poll(uproto_mqtt_t* client, uetd_u16_t work_budget);
uetd_status_t uproto_mqtt_next_event(uproto_mqtt_t* client, uproto_mqtt_event_t* event);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_MQTT_H */
