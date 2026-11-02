/// @file include/uproto/network/future/coap.h
/// @brief Fixed-bound CoAP request/response adapter contract.
///
/// CoAP parsing, token matching, confirmable retransmission, blockwise state,
/// and optional DTLS are provider responsibilities. The facade supports an
/// IP-stack provider or a native/external module stack while keeping request
/// and response storage caller-owned.

#ifndef UETD_UPROTO_FUTURE_COAP_H
#define UETD_UPROTO_FUTURE_COAP_H

#include "ucore/status.h"
#include "ucore/types.h"
#include "uproto/network/ip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPROTO_COAP_URI_MAX 255u
#define UPROTO_COAP_TOKEN_MAX 8u
#define UPROTO_COAP_PAYLOAD_MAX 512u
#define UPROTO_COAP_OPTION_MAX 8u

typedef enum {
    UPROTO_COAP_GET = 1,
    UPROTO_COAP_POST = 2,
    UPROTO_COAP_PUT = 3,
    UPROTO_COAP_DELETE = 4,
} uproto_coap_method_t;

typedef struct {
    uetd_u16_t number;
    const uetd_u8_t* value;
    uetd_u16_t length;
} uproto_coap_option_t;

typedef struct {
    uetd_u16_t number;
    uetd_u8_t length;
    uetd_u8_t value[64];
} uproto_coap_response_option_t;

typedef struct {
    uproto_ip_endpoint_t endpoint;
    uproto_coap_method_t method;
    const uetd_u8_t* uri_path;
    uetd_u16_t uri_path_length;
    const uetd_u8_t* token;
    uetd_u8_t token_length;
    const uproto_coap_option_t* options;
    uetd_u8_t option_count;
    const uetd_u8_t* payload;
    uetd_u16_t payload_length;
    bool confirmable;
    bool use_dtls;
    uetd_u16_t credential_handle;
} uproto_coap_request_t;

typedef struct {
    uetd_u8_t code_class;
    uetd_u8_t code_detail;
    uetd_u16_t message_id;
    uetd_u8_t token[UPROTO_COAP_TOKEN_MAX];
    uetd_u8_t token_length;
    uetd_u8_t option_count;
    uproto_coap_response_option_t options[UPROTO_COAP_OPTION_MAX];
    uetd_u16_t payload_length;
    uetd_u8_t payload[UPROTO_COAP_PAYLOAD_MAX];
} uproto_coap_response_t;

typedef struct {
    uetd_status_t (*request)(void* context, const uproto_coap_request_t* request, uproto_coap_response_t* response);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
} uproto_coap_ops_t;

typedef struct {
    const uproto_coap_ops_t* ops;
    void* context;
    bool bound;
} uproto_coap_t;

uetd_status_t uproto_coap_bind(uproto_coap_t* coap, const uproto_coap_ops_t* ops, void* context);
uetd_status_t uproto_coap_request(uproto_coap_t* coap, const uproto_coap_request_t* request, uproto_coap_response_t* response);
uetd_status_t uproto_coap_poll(uproto_coap_t* coap, uetd_u16_t work_budget);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_FUTURE_COAP_H */
