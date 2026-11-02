/// @file src/uproto/network/future/coap.c
/// @brief Validated dispatch for CoAP providers.
#include "uproto/network/future/coap.h"

static bool uproto_coap_family_valid(uproto_ip_family_t family) {
    return family == UPROTO_IP_FAMILY_IPV4 || family == UPROTO_IP_FAMILY_IPV6;
}

uetd_status_t uproto_coap_bind(uproto_coap_t* coap, const uproto_coap_ops_t* ops, void* context) {
    if (coap == NULL || ops == NULL) return UETD_INVALID_ARG;
    coap->ops = ops;
    coap->context = context;
    coap->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_coap_request(uproto_coap_t* coap, const uproto_coap_request_t* request, uproto_coap_response_t* response) {
    uetd_u8_t i;
    if (coap == NULL || !coap->bound || request == NULL || response == NULL || request->endpoint.port == 0u ||
        !uproto_coap_family_valid(request->endpoint.family) ||
        request->uri_path_length > UPROTO_COAP_URI_MAX || (request->uri_path_length != 0u && request->uri_path == NULL) ||
        request->token_length > UPROTO_COAP_TOKEN_MAX || (request->token_length != 0u && request->token == NULL) ||
        request->option_count > UPROTO_COAP_OPTION_MAX || (request->option_count != 0u && request->options == NULL) ||
        request->payload_length > UPROTO_COAP_PAYLOAD_MAX || (request->payload_length != 0u && request->payload == NULL) ||
        request->method < UPROTO_COAP_GET || request->method > UPROTO_COAP_DELETE) return UETD_INVALID_ARG;
    for (i = 0u; i < request->option_count; ++i) {
        if (request->options[i].length > 64u || (request->options[i].length != 0u && request->options[i].value == NULL))
            return UETD_INVALID_ARG;
    }
    if (coap->ops->request == NULL) return UETD_NOT_SUPPORTED;
    {
        uetd_status_t status = coap->ops->request(coap->context, request, response);
        if (status != UETD_OK) return status;
    }
    if (response->token_length > UPROTO_COAP_TOKEN_MAX || response->option_count > UPROTO_COAP_OPTION_MAX ||
        response->payload_length > UPROTO_COAP_PAYLOAD_MAX) return UETD_OVERFLOW;
    for (i = 0u; i < response->option_count; ++i) {
        if (response->options[i].length > sizeof(response->options[i].value)) return UETD_OVERFLOW;
    }
    return UETD_OK;
}

uetd_status_t uproto_coap_poll(uproto_coap_t* coap, uetd_u16_t work_budget) {
    if (coap == NULL || !coap->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (coap->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return coap->ops->poll(coap->context, work_budget);
}
