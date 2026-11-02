/// @file include/uproto/network/future/ppp.h
/// @brief Provider boundary for PPP-over-serial or native PPP links.
///
/// The selected provider owns PPP framing, LCP, authentication exchange,
/// IPv4CP/IPv6CP, retransmission timers, and the underlying UART/modem/SDK.
/// Credentials are opaque secure-store handles; this facade does not contain
/// a PPP implementation or retain raw secrets.

#ifndef UETD_UPROTO_FUTURE_PPP_H
#define UETD_UPROTO_FUTURE_PPP_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_PPP_MRU_MIN 128u
#define UPROTO_PPP_MRU_DEFAULT 1500u
#define UPROTO_PPP_MRU_MAX 1500u

enum {
    UPROTO_PPP_CAP_IPV4 = 1u << 0,
    UPROTO_PPP_CAP_IPV6 = 1u << 1,
    UPROTO_PPP_CAP_PAP = 1u << 2,
    UPROTO_PPP_CAP_CHAP = 1u << 3,
};

typedef enum {
    UPROTO_PPP_AUTH_NONE = 0,
    UPROTO_PPP_AUTH_PAP,
    UPROTO_PPP_AUTH_CHAP,
} uproto_ppp_auth_t;

typedef enum {
    UPROTO_PPP_STATE_STOPPED = 0,
    UPROTO_PPP_STATE_STARTING,
    UPROTO_PPP_STATE_AUTHENTICATING,
    UPROTO_PPP_STATE_NEGOTIATING,
    UPROTO_PPP_STATE_UP,
    UPROTO_PPP_STATE_ERROR,
} uproto_ppp_state_t;

typedef struct {
    uetd_u16_t maximum_mru;
    uproto_ppp_auth_t authentication;
    uetd_u16_t credential_handle; /* opaque secure-store/provider reference */
    bool request_ipv4;
    bool request_ipv6;
    uetd_u32_t connect_timeout_ms;
} uproto_ppp_config_t;

typedef struct {
    uproto_ppp_state_t state;
    uetd_u16_t negotiated_mru;
    bool ipv4_up;
    bool ipv6_up;
    uetd_u8_t reason_code;
    uetd_u32_t rx_dropped;
    uetd_u32_t tx_errors;
} uproto_ppp_status_t;

typedef struct {
    uetd_status_t (*start)(void* context, const uproto_ppp_config_t* config);
    uetd_status_t (*stop)(void* context);
    uetd_status_t (*get_status)(void* context, uproto_ppp_status_t* status);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
} uproto_ppp_ops_t;

typedef struct {
    const uproto_ppp_ops_t* ops;
    void* context;
    uetd_u32_t capabilities;
    uetd_u32_t requested_families;
    uetd_u16_t maximum_mru;
    bool bound;
} uproto_ppp_t;

uetd_status_t uproto_ppp_bind(uproto_ppp_t* ppp, const uproto_ppp_ops_t* ops, void* context,
                              uetd_u32_t capabilities);
uetd_status_t uproto_ppp_start(uproto_ppp_t* ppp, const uproto_ppp_config_t* config);
uetd_status_t uproto_ppp_stop(uproto_ppp_t* ppp);
uetd_status_t uproto_ppp_get_status(uproto_ppp_t* ppp, uproto_ppp_status_t* status);
uetd_status_t uproto_ppp_poll(uproto_ppp_t* ppp, uetd_u16_t work_budget);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_FUTURE_PPP_H */
