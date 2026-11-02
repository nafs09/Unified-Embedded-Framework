/// @file src/uproto/network/http.c
/// @brief Validated dispatch for HTTP providers.
#include "uproto/network/http.h"

uetd_status_t uproto_http_bind(uproto_http_t* http, const uproto_http_ops_t* ops, void* context) {
    if (http == NULL || ops == NULL) return UETD_INVALID_ARG;
    http->ops = ops;
    http->context = context;
    http->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_http_perform(uproto_http_t* http, const uproto_http_request_t* request, uproto_http_response_t* response) {
    uetd_u8_t i;
    if (http == NULL || !http->bound || request == NULL || response == NULL || request->uri == NULL || request->uri_length == 0u ||
        request->uri_length > UPROTO_HTTP_URI_MAX || request->method < UPROTO_HTTP_GET || request->method > UPROTO_HTTP_HEAD ||
        request->timeout_ms == 0u ||
        request->header_count > UPROTO_HTTP_HEADER_COUNT_MAX || (request->header_count != 0u && request->headers == NULL) ||
        (request->body_length != 0u && request->body == NULL) || (response->body_capacity != 0u && response->body == NULL))
        return UETD_INVALID_ARG;
    for (i = 0u; i < request->header_count; ++i) {
        if (request->headers[i].name == NULL || request->headers[i].name_length == 0u ||
            request->headers[i].name_length > UPROTO_HTTP_HEADER_NAME_MAX ||
            request->headers[i].value_length > UPROTO_HTTP_HEADER_VALUE_MAX ||
            (request->headers[i].value_length != 0u && request->headers[i].value == NULL)) return UETD_INVALID_ARG;
    }
    if (response->header_capacity > UPROTO_HTTP_HEADER_COUNT_MAX ||
        (response->header_capacity != 0u && response->headers == NULL)) return UETD_INVALID_ARG;
    for (i = 0u; i < response->header_capacity; ++i) {
        if ((response->headers[i].name_capacity != 0u && response->headers[i].name == NULL) ||
            (response->headers[i].value_capacity != 0u && response->headers[i].value == NULL)) return UETD_INVALID_ARG;
    }
    if (http->ops->perform == NULL) return UETD_NOT_SUPPORTED;
    {
        uetd_status_t status = http->ops->perform(http->context, request, response);
        if (status != UETD_OK) return status;
    }
    if (response->body_length > response->body_capacity || response->header_count > response->header_capacity)
        return UETD_OVERFLOW;
    for (i = 0u; i < response->header_count; ++i) {
        if (response->headers[i].name_length > response->headers[i].name_capacity ||
            response->headers[i].value_length > response->headers[i].value_capacity) return UETD_OVERFLOW;
    }
    return UETD_OK;
}

uetd_status_t uproto_http_cancel(uproto_http_t* http) {
    if (http == NULL || !http->bound) return UETD_INVALID_ARG;
    if (http->ops->cancel == NULL) return UETD_NOT_SUPPORTED;
    return http->ops->cancel(http->context);
}
