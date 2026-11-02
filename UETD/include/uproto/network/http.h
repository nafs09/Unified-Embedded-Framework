/// @file include/uproto/network/http.h
/// @brief Caller-buffered HTTP client adapter contract.
///
/// The provider implements HTTP framing, redirects policy, connection reuse,
/// timeout handling, and TLS when explicitly configured. Providers may use
/// UPROTO IP sockets, an integrated SDK, or a network coprocessor's HTTP
/// offload. HTTP/TLS is not available until a provider is supplied.

#ifndef UETD_UPROTO_HTTP_H
#define UETD_UPROTO_HTTP_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_HTTP_URI_MAX 255u
#define UPROTO_HTTP_HEADER_COUNT_MAX 8u
#define UPROTO_HTTP_HEADER_NAME_MAX 32u
#define UPROTO_HTTP_HEADER_VALUE_MAX 96u

typedef enum {
    UPROTO_HTTP_GET = 0,
    UPROTO_HTTP_POST,
    UPROTO_HTTP_PUT,
    UPROTO_HTTP_DELETE,
    UPROTO_HTTP_HEAD,
} uproto_http_method_t;

typedef struct {
    const uetd_u8_t* name;
    uetd_u8_t name_length;
    const uetd_u8_t* value;
    uetd_u8_t value_length;
} uproto_http_header_t;

typedef struct {
    uetd_u8_t* name;
    uetd_u8_t name_capacity;
    uetd_u8_t name_length;
    uetd_u8_t* value;
    uetd_u8_t value_capacity;
    uetd_u8_t value_length;
} uproto_http_response_header_t;

typedef struct {
    uproto_http_method_t method;
    const uetd_u8_t* uri;
    uetd_u16_t uri_length;
    const uproto_http_header_t* headers;
    uetd_u8_t header_count;
    const uetd_u8_t* body;
    uetd_u16_t body_length;
    bool use_tls;
    uetd_u16_t credential_handle;
    uetd_u32_t timeout_ms;
} uproto_http_request_t;

typedef struct {
    uetd_u16_t status_code;
    uetd_u16_t body_length;
    uetd_u8_t* body;
    uetd_u16_t body_capacity;
    uproto_http_response_header_t* headers;
    uetd_u8_t header_capacity;
    uetd_u8_t header_count;
    uetd_u32_t flags; /* provider-defined truncation/redirect/connection details */
} uproto_http_response_t;

typedef struct {
    uetd_status_t (*perform)(void* context, const uproto_http_request_t* request, uproto_http_response_t* response);
    uetd_status_t (*cancel)(void* context);
} uproto_http_ops_t;

typedef struct {
    const uproto_http_ops_t* ops;
    void* context;
    bool bound;
} uproto_http_t;

uetd_status_t uproto_http_bind(uproto_http_t* http, const uproto_http_ops_t* ops, void* context);
uetd_status_t uproto_http_perform(uproto_http_t* http, const uproto_http_request_t* request, uproto_http_response_t* response);
uetd_status_t uproto_http_cancel(uproto_http_t* http);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_HTTP_H */
