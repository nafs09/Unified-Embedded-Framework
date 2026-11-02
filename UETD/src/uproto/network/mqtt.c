/// @file src/uproto/network/mqtt.c
/// @brief Validated dispatch for MQTT providers.
#include "uproto/network/mqtt.h"

static bool uproto_mqtt_qos_valid(uproto_mqtt_qos_t qos) { return qos >= UPROTO_MQTT_QOS_0 && qos <= UPROTO_MQTT_QOS_2; }

uetd_status_t uproto_mqtt_bind(uproto_mqtt_t* client, const uproto_mqtt_ops_t* ops, void* context) {
    if (client == NULL || ops == NULL) return UETD_INVALID_ARG;
    client->ops = ops;
    client->context = context;
    client->maximum_packet_bytes = 0u;
    client->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_mqtt_connect(uproto_mqtt_t* client, const uproto_mqtt_config_t* config) {
    if (client == NULL || !client->bound || config == NULL ||
        (config->version != UPROTO_MQTT_VERSION_3_1_1 && config->version != UPROTO_MQTT_VERSION_5) ||
        config->broker.port == 0u || config->client_id == NULL || config->client_id_length == 0u ||
        config->maximum_packet_bytes == 0u || config->maximum_packet_bytes > UPROTO_MQTT_PACKET_MAX ||
        (config->broker.family != UPROTO_IP_FAMILY_IPV4 && config->broker.family != UPROTO_IP_FAMILY_IPV6)) return UETD_INVALID_ARG;
    if (client->ops->connect == NULL) return UETD_NOT_SUPPORTED;
    {
        uetd_status_t status = client->ops->connect(client->context, config);
        if (status == UETD_OK) client->maximum_packet_bytes = config->maximum_packet_bytes;
        return status;
    }
}

uetd_status_t uproto_mqtt_disconnect(uproto_mqtt_t* client) {
    if (client == NULL || !client->bound) return UETD_INVALID_ARG;
    if (client->ops->disconnect == NULL) return UETD_NOT_SUPPORTED;
    {
        uetd_status_t status = client->ops->disconnect(client->context);
        if (status == UETD_OK) client->maximum_packet_bytes = 0u;
        return status;
    }
}

uetd_status_t uproto_mqtt_publish(uproto_mqtt_t* client, const uetd_u8_t* topic, uetd_u16_t topic_length,
                                  const uetd_u8_t* payload, uetd_u16_t payload_length, uproto_mqtt_qos_t qos, bool retain) {
    if (client == NULL || !client->bound || client->maximum_packet_bytes == 0u || topic == NULL || topic_length == 0u ||
        topic_length > UPROTO_MQTT_TOPIC_MAX || (uetd_u32_t)topic_length + payload_length > client->maximum_packet_bytes ||
        (payload_length != 0u && payload == NULL) || !uproto_mqtt_qos_valid(qos)) return UETD_INVALID_ARG;
    if (client->ops->publish == NULL) return UETD_NOT_SUPPORTED;
    return client->ops->publish(client->context, topic, topic_length, payload, payload_length, qos, retain);
}

uetd_status_t uproto_mqtt_subscribe(uproto_mqtt_t* client, const uetd_u8_t* topic_filter, uetd_u16_t topic_length,
                                    uproto_mqtt_qos_t qos) {
    if (client == NULL || !client->bound || client->maximum_packet_bytes == 0u || topic_filter == NULL || topic_length == 0u ||
        topic_length > UPROTO_MQTT_TOPIC_MAX || topic_length > client->maximum_packet_bytes ||
        !uproto_mqtt_qos_valid(qos)) return UETD_INVALID_ARG;
    if (client->ops->subscribe == NULL) return UETD_NOT_SUPPORTED;
    return client->ops->subscribe(client->context, topic_filter, topic_length, qos);
}

uetd_status_t uproto_mqtt_unsubscribe(uproto_mqtt_t* client, const uetd_u8_t* topic_filter, uetd_u16_t topic_length) {
    if (client == NULL || !client->bound || client->maximum_packet_bytes == 0u || topic_filter == NULL || topic_length == 0u ||
        topic_length > UPROTO_MQTT_TOPIC_MAX || topic_length > client->maximum_packet_bytes)
        return UETD_INVALID_ARG;
    if (client->ops->unsubscribe == NULL) return UETD_NOT_SUPPORTED;
    return client->ops->unsubscribe(client->context, topic_filter, topic_length);
}

uetd_status_t uproto_mqtt_poll(uproto_mqtt_t* client, uetd_u16_t work_budget) {
    if (client == NULL || !client->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (client->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return client->ops->poll(client->context, work_budget);
}

uetd_status_t uproto_mqtt_next_event(uproto_mqtt_t* client, uproto_mqtt_event_t* event) {
    uetd_status_t status;
    if (client == NULL || !client->bound || event == NULL) return UETD_INVALID_ARG;
    if (client->ops->next_event == NULL) return UETD_NOT_SUPPORTED;
    status = client->ops->next_event(client->context, event);
    if (status != UETD_OK) return status;
    if (event->topic_length > UPROTO_MQTT_TOPIC_MAX || event->payload_length > UPROTO_MQTT_EVENT_DATA_MAX) return UETD_OVERFLOW;
    if (event->kind < UPROTO_MQTT_EVENT_CONNECTED || event->kind > UPROTO_MQTT_EVENT_ERROR) return UETD_ERROR;
    return UETD_OK;
}
