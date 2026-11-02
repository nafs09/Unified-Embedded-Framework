/// @file include/uproto/network/tls.h
/// @brief External TLS/DTLS provider facade with opaque credential handles.
///
/// Cryptographic implementation, trust-store ownership, entropy, certificate
/// parsing, and hardware key access remain provider dependencies. UETD never
/// accepts raw private-key material through this API and does not claim TLS
/// support without a bound, qualified provider.

#ifndef UETD_UPROTO_TLS_H
#define UETD_UPROTO_TLS_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "uproto/network/ip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_TLS_RECORD_MAX 16384u

typedef enum {
    UPROTO_TLS_PROFILE_TLS_1_2 = 0,
    UPROTO_TLS_PROFILE_TLS_1_3,
    UPROTO_TLS_PROFILE_DTLS_1_2,
} uproto_tls_profile_t;

typedef struct {
    uproto_tls_profile_t profile;
    uproto_ip_endpoint_t peer;
    const uetd_u8_t* server_name;
    uetd_u8_t server_name_length;
    uetd_u16_t trust_store_handle;
    uetd_u16_t client_identity_handle;
    uetd_u16_t maximum_record_bytes;
    uetd_u32_t handshake_timeout_ms;
    bool require_peer_verification;
} uproto_tls_config_t;

typedef struct {
    uetd_u32_t handle;
    uetd_u16_t maximum_record_bytes;
    bool active;
} uproto_tls_session_t;

typedef struct {
    uetd_status_t (*handshake)(void* context, const uproto_tls_config_t* config, uetd_u32_t* session_handle);
    uetd_status_t (*write)(void* context, uetd_u32_t session_handle, const uetd_u8_t* bytes,
                           uetd_u16_t length, uetd_u16_t* written);
    uetd_status_t (*read)(void* context, uetd_u32_t session_handle, uetd_u8_t* bytes,
                          uetd_u16_t capacity, uetd_u16_t* received);
    uetd_status_t (*close)(void* context, uetd_u32_t session_handle);
} uproto_tls_ops_t;

typedef struct {
    const uproto_tls_ops_t* ops;
    void* context;
    bool bound;
} uproto_tls_t;

uetd_status_t uproto_tls_bind(uproto_tls_t* tls, const uproto_tls_ops_t* ops, void* context);
uetd_status_t uproto_tls_handshake(uproto_tls_t* tls, const uproto_tls_config_t* config, uproto_tls_session_t* session);
uetd_status_t uproto_tls_write(uproto_tls_t* tls, uproto_tls_session_t* session, const uetd_u8_t* bytes,
                               uetd_u16_t length, uetd_u16_t* written);
uetd_status_t uproto_tls_read(uproto_tls_t* tls, uproto_tls_session_t* session, uetd_u8_t* bytes,
                              uetd_u16_t capacity, uetd_u16_t* received);
uetd_status_t uproto_tls_close(uproto_tls_t* tls, uproto_tls_session_t* session);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_TLS_H */
