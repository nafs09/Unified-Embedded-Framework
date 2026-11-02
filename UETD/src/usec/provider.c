/// @file src/usec/provider.c
/// @brief Source scaffold; all operations fail closed without side effects.
#include "usec/provider.h"

uetd_status_t usec_init(usec_provider_t* security, const usec_config_t* config) {
    /* TODO: Bind a vetted pinned provider, declared algorithms, trust/key storage and production entropy health policy. */
    (void)security;
    (void)config;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t usec_random(usec_provider_t* security, ucore_write_span_t output) {
    /* TODO: Require entropy/DRBG readiness and a bounded request; simulation data must not masquerade as production entropy. */
    (void)security;
    (void)output;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t usec_execute(usec_provider_t* security, const usec_request_t* request, ucore_write_span_t output, size_t* written) {
    /* TODO: Validate algorithm-specific lengths/key use and constant-time provider requirements; authenticated decryption cannot publish unauthenticated plaintext. */
    (void)security;
    (void)request;
    (void)output;
    (void)written;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t usec_authorize(usec_provider_t* security, const usec_authorization_t* request) {
    /* TODO: Resolve principal/capability/target/context using product policy; default denial must never be reported as permission. */
    (void)security;
    (void)request;
    return UETD_NOT_SUPPORTED;
}

uetd_status_t usec_key_destroy(usec_provider_t* security, usec_key_handle_t key) {
    /* TODO: Validate key generation/use policy, quiesce users and invalidate the handle without leaking material. */
    (void)security;
    (void)key;
    return UETD_NOT_SUPPORTED;
}

