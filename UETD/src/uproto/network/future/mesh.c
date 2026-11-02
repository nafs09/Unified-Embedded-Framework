/// @file src/uproto/network/future/mesh.c
/// @brief Validated dispatch for Thread/Zigbee provider adapters.
#include "uproto/network/future/mesh.h"

uetd_status_t uproto_mesh_bind(uproto_mesh_t* mesh, uproto_mesh_profile_t profile, const uproto_mesh_ops_t* ops, void* context) {
    if (mesh == NULL || ops == NULL || (profile != UPROTO_MESH_PROFILE_THREAD && profile != UPROTO_MESH_PROFILE_ZIGBEE))
        return UETD_INVALID_ARG;
    mesh->ops = ops;
    mesh->context = context;
    mesh->profile = profile;
    mesh->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_mesh_start(uproto_mesh_t* mesh, const uproto_mesh_config_t* config, bool form_network) {
    if (mesh == NULL || !mesh->bound || config == NULL || config->profile != mesh->profile ||
        config->network_name_length > UPROTO_MESH_NAME_MAX ||
        (config->channel != 0u && (config->channel < 11u || config->channel > 26u)) ||
        (config->role < UPROTO_MESH_ROLE_END_DEVICE || config->role > UPROTO_MESH_ROLE_COORDINATOR)) return UETD_INVALID_ARG;
    if ((mesh->profile == UPROTO_MESH_PROFILE_THREAD && config->role == UPROTO_MESH_ROLE_COORDINATOR) ||
        (mesh->profile == UPROTO_MESH_PROFILE_ZIGBEE && config->role == UPROTO_MESH_ROLE_COMMISSIONER)) return UETD_INVALID_ARG;
    if (mesh->ops->start == NULL) return UETD_NOT_SUPPORTED;
    return mesh->ops->start(mesh->context, config, form_network);
}

uetd_status_t uproto_mesh_permit_join(uproto_mesh_t* mesh, uetd_u16_t duration_seconds) {
    if (mesh == NULL || !mesh->bound || duration_seconds == 0u) return UETD_INVALID_ARG;
    if (mesh->profile != UPROTO_MESH_PROFILE_ZIGBEE || mesh->ops->permit_join == NULL) return UETD_NOT_SUPPORTED;
    return mesh->ops->permit_join(mesh->context, duration_seconds);
}

uetd_status_t uproto_mesh_remove_device(uproto_mesh_t* mesh, uetd_u64_t peer_id) {
    if (mesh == NULL || !mesh->bound || peer_id == 0u) return UETD_INVALID_ARG;
    if (mesh->profile != UPROTO_MESH_PROFILE_ZIGBEE || mesh->ops->remove_device == NULL) return UETD_NOT_SUPPORTED;
    return mesh->ops->remove_device(mesh->context, peer_id);
}

uetd_status_t uproto_mesh_send_zigbee_command(uproto_mesh_t* mesh, uetd_u64_t peer_id, uetd_u8_t endpoint,
                                              uetd_u16_t cluster_id, uetd_u16_t command_id,
                                              const uetd_u8_t* payload, uetd_u16_t payload_length) {
    if (mesh == NULL || !mesh->bound || peer_id == 0u || endpoint == 0u || payload_length > UPROTO_MESH_EVENT_DATA_MAX ||
        (payload_length != 0u && payload == NULL)) return UETD_INVALID_ARG;
    if (mesh->profile != UPROTO_MESH_PROFILE_ZIGBEE || mesh->ops->send_zigbee_command == NULL) return UETD_NOT_SUPPORTED;
    return mesh->ops->send_zigbee_command(mesh->context, peer_id, endpoint, cluster_id, command_id, payload, payload_length);
}

uetd_status_t uproto_mesh_leave(uproto_mesh_t* mesh) {
    if (mesh == NULL || !mesh->bound) return UETD_INVALID_ARG;
    if (mesh->ops->leave == NULL) return UETD_NOT_SUPPORTED;
    return mesh->ops->leave(mesh->context);
}

uetd_status_t uproto_mesh_get_state(uproto_mesh_t* mesh, uproto_mesh_state_t* state) {
    uproto_mesh_state_t actual;
    uetd_status_t status;
    if (mesh == NULL || !mesh->bound || state == NULL) return UETD_INVALID_ARG;
    if (mesh->ops->get_state == NULL) return UETD_NOT_SUPPORTED;
    status = mesh->ops->get_state(mesh->context, &actual);
    if (status != UETD_OK) return status;
    if (actual < UPROTO_MESH_STATE_STOPPED || actual > UPROTO_MESH_STATE_ERROR) return UETD_ERROR;
    *state = actual;
    return UETD_OK;
}

uetd_status_t uproto_mesh_poll(uproto_mesh_t* mesh, uetd_u16_t work_budget) {
    if (mesh == NULL || !mesh->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (mesh->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return mesh->ops->poll(mesh->context, work_budget);
}

uetd_status_t uproto_mesh_next_event(uproto_mesh_t* mesh, uproto_mesh_event_t* event) {
    uetd_status_t status;
    if (mesh == NULL || !mesh->bound || event == NULL) return UETD_INVALID_ARG;
    if (mesh->ops->next_event == NULL) return UETD_NOT_SUPPORTED;
    status = mesh->ops->next_event(mesh->context, event);
    if (status != UETD_OK) return status;
    if (event->kind < UPROTO_MESH_EVENT_STATE || event->kind > UPROTO_MESH_EVENT_ERROR ||
        event->profile != mesh->profile) return UETD_ERROR;
    if (event->data_length > UPROTO_MESH_EVENT_DATA_MAX) return UETD_OVERFLOW;
    return UETD_OK;
}
