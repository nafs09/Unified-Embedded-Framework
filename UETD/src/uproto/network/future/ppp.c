/// @file src/uproto/network/future/ppp.c
/// @brief Validated dispatch for external and native PPP providers.
#include "uproto/network/future/ppp.h"

static bool uproto_ppp_auth_valid(uproto_ppp_auth_t authentication) {
    return authentication >= UPROTO_PPP_AUTH_NONE && authentication <= UPROTO_PPP_AUTH_CHAP;
}

uetd_status_t uproto_ppp_bind(uproto_ppp_t* ppp, const uproto_ppp_ops_t* ops, void* context,
                              uetd_u32_t capabilities) {
    const uetd_u32_t known = UPROTO_PPP_CAP_IPV4 | UPROTO_PPP_CAP_IPV6 | UPROTO_PPP_CAP_PAP | UPROTO_PPP_CAP_CHAP;
    if (ppp == NULL || ops == NULL || capabilities == 0u || (capabilities & ~known) != 0u ||
        (capabilities & (UPROTO_PPP_CAP_IPV4 | UPROTO_PPP_CAP_IPV6)) == 0u) return UETD_INVALID_ARG;
    ppp->ops = ops;
    ppp->context = context;
    ppp->capabilities = capabilities;
    ppp->requested_families = 0u;
    ppp->maximum_mru = 0u;
    ppp->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_ppp_start(uproto_ppp_t* ppp, const uproto_ppp_config_t* config) {
    uetd_u32_t requested_families = 0u;
    uetd_status_t status;
    if (ppp == NULL || !ppp->bound || config == NULL || config->maximum_mru < UPROTO_PPP_MRU_MIN ||
        config->maximum_mru > UPROTO_PPP_MRU_MAX || !uproto_ppp_auth_valid(config->authentication) ||
        config->connect_timeout_ms == 0u || (!config->request_ipv4 && !config->request_ipv6)) return UETD_INVALID_ARG;
    if (config->request_ipv4) requested_families |= UPROTO_PPP_CAP_IPV4;
    if (config->request_ipv6) requested_families |= UPROTO_PPP_CAP_IPV6;
    if ((requested_families & ppp->capabilities) != requested_families) return UETD_NOT_SUPPORTED;
    if ((config->authentication == UPROTO_PPP_AUTH_PAP && (ppp->capabilities & UPROTO_PPP_CAP_PAP) == 0u) ||
        (config->authentication == UPROTO_PPP_AUTH_CHAP && (ppp->capabilities & UPROTO_PPP_CAP_CHAP) == 0u))
        return UETD_NOT_SUPPORTED;
    if ((config->authentication == UPROTO_PPP_AUTH_NONE && config->credential_handle != 0u) ||
        (config->authentication != UPROTO_PPP_AUTH_NONE && config->credential_handle == 0u)) return UETD_INVALID_ARG;
    if (ppp->ops->start == NULL) return UETD_NOT_SUPPORTED;
    status = ppp->ops->start(ppp->context, config);
    if (status == UETD_OK) {
        ppp->requested_families = requested_families;
        ppp->maximum_mru = config->maximum_mru;
    }
    return status;
}

uetd_status_t uproto_ppp_stop(uproto_ppp_t* ppp) {
    uetd_status_t status;
    if (ppp == NULL || !ppp->bound) return UETD_INVALID_ARG;
    if (ppp->ops->stop == NULL) return UETD_NOT_SUPPORTED;
    status = ppp->ops->stop(ppp->context);
    if (status == UETD_OK) {
        ppp->requested_families = 0u;
        ppp->maximum_mru = 0u;
    }
    return status;
}

uetd_status_t uproto_ppp_get_status(uproto_ppp_t* ppp, uproto_ppp_status_t* status) {
    uproto_ppp_status_t actual;
    uetd_status_t result;
    if (ppp == NULL || !ppp->bound || status == NULL) return UETD_INVALID_ARG;
    if (ppp->ops->get_status == NULL) return UETD_NOT_SUPPORTED;
    result = ppp->ops->get_status(ppp->context, &actual);
    if (result != UETD_OK) return result;
    if (actual.state < UPROTO_PPP_STATE_STOPPED || actual.state > UPROTO_PPP_STATE_ERROR ||
        (actual.negotiated_mru != 0u &&
         (actual.negotiated_mru < UPROTO_PPP_MRU_MIN || actual.negotiated_mru > ppp->maximum_mru)) ||
        (actual.ipv4_up && (ppp->requested_families & UPROTO_PPP_CAP_IPV4) == 0u) ||
        (actual.ipv6_up && (ppp->requested_families & UPROTO_PPP_CAP_IPV6) == 0u) ||
        (actual.state == UPROTO_PPP_STATE_UP && !actual.ipv4_up && !actual.ipv6_up)) return UETD_ERROR;
    *status = actual;
    return UETD_OK;
}

uetd_status_t uproto_ppp_poll(uproto_ppp_t* ppp, uetd_u16_t work_budget) {
    if (ppp == NULL || !ppp->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (ppp->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return ppp->ops->poll(ppp->context, work_budget);
}
