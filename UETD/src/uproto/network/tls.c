/// @file src/uproto/network/tls.c
/// @brief Validated dispatch for TLS/DTLS providers.
#include "uproto/network/tls.h"

uetd_status_t uproto_tls_bind(uproto_tls_t* tls, const uproto_tls_ops_t* ops, void* context) {
    if (tls == NULL || ops == NULL) return UETD_INVALID_ARG;
    tls->ops = ops;
    tls->context = context;
    tls->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_tls_handshake(uproto_tls_t* tls, const uproto_tls_config_t* config, uproto_tls_session_t* session) {
    uetd_u32_t handle = 0u;
    uetd_status_t status;
    if (tls == NULL || !tls->bound || config == NULL || session == NULL || config->peer.port == 0u ||
        (config->peer.family != UPROTO_IP_FAMILY_IPV4 && config->peer.family != UPROTO_IP_FAMILY_IPV6) ||
        config->server_name_length > 253u || (config->server_name_length != 0u && config->server_name == NULL) ||
        config->maximum_record_bytes == 0u || config->maximum_record_bytes > UPROTO_TLS_RECORD_MAX || config->handshake_timeout_ms == 0u ||
        !config->require_peer_verification ||
        config->profile < UPROTO_TLS_PROFILE_TLS_1_2 || config->profile > UPROTO_TLS_PROFILE_DTLS_1_2)
        return UETD_INVALID_ARG;
    if (tls->ops->handshake == NULL) return UETD_NOT_SUPPORTED;
    status = tls->ops->handshake(tls->context, config, &handle);
    if (status != UETD_OK) return status;
    session->handle = handle;
    session->maximum_record_bytes = config->maximum_record_bytes;
    session->active = true;
    return UETD_OK;
}

uetd_status_t uproto_tls_write(uproto_tls_t* tls, uproto_tls_session_t* session, const uetd_u8_t* bytes,
                               uetd_u16_t length, uetd_u16_t* written) {
    uetd_u16_t actual = 0u;
    uetd_status_t status;
    if (tls == NULL || !tls->bound || session == NULL || !session->active || bytes == NULL || length == 0u ||
        length > session->maximum_record_bytes || written == NULL)
        return UETD_INVALID_ARG;
    if (tls->ops->write == NULL) return UETD_NOT_SUPPORTED;
    status = tls->ops->write(tls->context, session->handle, bytes, length, &actual);
    if (status != UETD_OK) return status;
    if (actual > length) return UETD_OVERFLOW;
    *written = actual;
    return UETD_OK;
}

uetd_status_t uproto_tls_read(uproto_tls_t* tls, uproto_tls_session_t* session, uetd_u8_t* bytes,
                              uetd_u16_t capacity, uetd_u16_t* received) {
    uetd_u16_t actual = 0u;
    uetd_status_t status;
    if (tls == NULL || !tls->bound || session == NULL || !session->active || bytes == NULL || capacity == 0u ||
        capacity > session->maximum_record_bytes || received == NULL)
        return UETD_INVALID_ARG;
    if (tls->ops->read == NULL) return UETD_NOT_SUPPORTED;
    status = tls->ops->read(tls->context, session->handle, bytes, capacity, &actual);
    if (status != UETD_OK) return status;
    if (actual > capacity) return UETD_OVERFLOW;
    *received = actual;
    return actual == 0u ? UETD_NOT_READY : UETD_OK;
}

uetd_status_t uproto_tls_close(uproto_tls_t* tls, uproto_tls_session_t* session) {
    uetd_status_t status;
    if (tls == NULL || !tls->bound || session == NULL || !session->active) return UETD_INVALID_ARG;
    if (tls->ops->close == NULL) return UETD_NOT_SUPPORTED;
    status = tls->ops->close(tls->context, session->handle);
    if (status == UETD_OK) {
        session->active = false;
        session->maximum_record_bytes = 0u;
    }
    return status;
}
