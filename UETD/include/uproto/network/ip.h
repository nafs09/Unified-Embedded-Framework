/// @file include/uproto/network/ip.h
/// @brief Bounded adapter boundary for an IP stack and socket provider.
///
/// This is the network service surface used by wired and wireless links. A
/// provider may bind a UETD-owned stack, an integrated SoC stack, or a network
/// coprocessor's socket offload. UETD does not silently supply DHCP,
/// DNS, TCP, UDP, IPv4, IPv6, or security guarantees when no provider is bound.

#ifndef UETD_UPROTO_IP_H
#define UETD_UPROTO_IP_H

#include "ucore/status.h"
#include "ucore/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UPROTO_IP_FAMILY_IPV4 = 4,
    UPROTO_IP_FAMILY_IPV6 = 6,
} uproto_ip_family_t;

typedef enum {
    UPROTO_IP_SOCKET_DATAGRAM = 0,
    UPROTO_IP_SOCKET_STREAM,
} uproto_ip_socket_kind_t;

enum {
    UPROTO_IP_CAP_IPV4 = 1u << 0,
    UPROTO_IP_CAP_IPV6 = 1u << 1,
    UPROTO_IP_CAP_TCP = 1u << 2,
    UPROTO_IP_CAP_UDP = 1u << 3,
    UPROTO_IP_CAP_DHCP = 1u << 4,
    UPROTO_IP_CAP_DNS = 1u << 5,
    UPROTO_IP_CAP_SOCKET_OFFLOAD = 1u << 6,
};

typedef struct {
    uproto_ip_family_t family;
    uetd_u8_t address[16];
    uetd_u16_t port;
    uetd_u32_t scope_id; /* IPv6 interface scope; zero when not applicable */
} uproto_ip_endpoint_t;

typedef struct {
    uetd_u32_t handle;
    uproto_ip_socket_kind_t kind;
    uproto_ip_family_t family;
    bool active;
} uproto_ip_socket_t;

typedef struct {
    uproto_ip_family_t family;
    uetd_u8_t address[16];
    uetd_u8_t prefix_length;
    uetd_u8_t gateway[16];
    uetd_u8_t dns_server[16];
} uproto_ip_interface_config_t;

typedef struct {
    uetd_status_t (*dhcp_start)(void* context, uetd_u8_t interface_id);
    uetd_status_t (*dhcp_stop)(void* context, uetd_u8_t interface_id);
    uetd_status_t (*ipv6_autoconfigure)(void* context, uetd_u8_t interface_id, bool enable_slaac);
    uetd_status_t (*configure_interface)(void* context, uetd_u8_t interface_id, const uproto_ip_interface_config_t* config);
    uetd_status_t (*get_interface_config)(void* context, uetd_u8_t interface_id, uproto_ip_interface_config_t* config);
    uetd_status_t (*dns_resolve)(void* context, const uetd_u8_t* hostname, uetd_u8_t hostname_length,
                                 uproto_ip_family_t family, uproto_ip_endpoint_t* result);
    uetd_status_t (*socket_open)(void* context, uproto_ip_family_t family, uproto_ip_socket_kind_t kind, uetd_u32_t* handle);
    uetd_status_t (*socket_connect)(void* context, uetd_u32_t handle, const uproto_ip_endpoint_t* endpoint);
    uetd_status_t (*socket_bind)(void* context, uetd_u32_t handle, const uproto_ip_endpoint_t* endpoint);
    uetd_status_t (*socket_listen)(void* context, uetd_u32_t handle, uetd_u8_t backlog);
    uetd_status_t (*socket_accept)(void* context, uetd_u32_t handle, uetd_u32_t* accepted_handle,
                                   uproto_ip_endpoint_t* peer);
    uetd_status_t (*socket_send)(void* context, uetd_u32_t handle, const uetd_u8_t* bytes, uetd_u16_t length,
                                 uetd_u16_t* sent);
    uetd_status_t (*socket_receive)(void* context, uetd_u32_t handle, uetd_u8_t* bytes, uetd_u16_t capacity,
                                    uetd_u16_t* received);
    uetd_status_t (*socket_send_to)(void* context, uetd_u32_t handle, const uproto_ip_endpoint_t* peer,
                                    const uetd_u8_t* bytes, uetd_u16_t length, uetd_u16_t* sent);
    uetd_status_t (*socket_receive_from)(void* context, uetd_u32_t handle, uproto_ip_endpoint_t* peer,
                                         uetd_u8_t* bytes, uetd_u16_t capacity, uetd_u16_t* received);
    uetd_status_t (*socket_close)(void* context, uetd_u32_t handle);
    uetd_status_t (*poll)(void* context, uetd_u16_t work_budget);
} uproto_ip_ops_t;

typedef struct {
    const uproto_ip_ops_t* ops;
    void* context;
    uetd_u32_t capabilities;
    bool bound;
} uproto_ip_t;

uetd_status_t uproto_ip_bind(uproto_ip_t* ip, const uproto_ip_ops_t* ops, void* context, uetd_u32_t capabilities);
uetd_status_t uproto_ip_dhcp_start(uproto_ip_t* ip, uetd_u8_t interface_id);
uetd_status_t uproto_ip_dhcp_stop(uproto_ip_t* ip, uetd_u8_t interface_id);
uetd_status_t uproto_ip_ipv6_autoconfigure(uproto_ip_t* ip, uetd_u8_t interface_id, bool enable_slaac);
uetd_status_t uproto_ip_configure_interface(uproto_ip_t* ip, uetd_u8_t interface_id,
                                            const uproto_ip_interface_config_t* config);
uetd_status_t uproto_ip_get_interface_config(uproto_ip_t* ip, uetd_u8_t interface_id,
                                              uproto_ip_interface_config_t* config);
uetd_status_t uproto_ip_dns_resolve(uproto_ip_t* ip, const uetd_u8_t* hostname, uetd_u8_t hostname_length,
                                    uproto_ip_family_t family, uproto_ip_endpoint_t* result);
uetd_status_t uproto_ip_socket_open(uproto_ip_t* ip, uproto_ip_family_t family, uproto_ip_socket_kind_t kind,
                                    uproto_ip_socket_t* socket);
uetd_status_t uproto_ip_socket_connect(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uproto_ip_endpoint_t* endpoint);
uetd_status_t uproto_ip_socket_bind(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uproto_ip_endpoint_t* endpoint);
uetd_status_t uproto_ip_socket_listen(uproto_ip_t* ip, uproto_ip_socket_t* socket, uetd_u8_t backlog);
uetd_status_t uproto_ip_socket_accept(uproto_ip_t* ip, uproto_ip_socket_t* listener, uproto_ip_socket_t* accepted,
                                      uproto_ip_endpoint_t* peer);
uetd_status_t uproto_ip_socket_send(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uetd_u8_t* bytes,
                                    uetd_u16_t length, uetd_u16_t* sent);
uetd_status_t uproto_ip_socket_receive(uproto_ip_t* ip, uproto_ip_socket_t* socket, uetd_u8_t* bytes,
                                       uetd_u16_t capacity, uetd_u16_t* received);
uetd_status_t uproto_ip_socket_send_to(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uproto_ip_endpoint_t* peer,
                                       const uetd_u8_t* bytes, uetd_u16_t length, uetd_u16_t* sent);
uetd_status_t uproto_ip_socket_receive_from(uproto_ip_t* ip, uproto_ip_socket_t* socket, uproto_ip_endpoint_t* peer,
                                            uetd_u8_t* bytes, uetd_u16_t capacity, uetd_u16_t* received);
uetd_status_t uproto_ip_socket_close(uproto_ip_t* ip, uproto_ip_socket_t* socket);
uetd_status_t uproto_ip_poll(uproto_ip_t* ip, uetd_u16_t work_budget);

#ifdef __cplusplus
}
#endif

#endif /* UETD_UPROTO_IP_H */
