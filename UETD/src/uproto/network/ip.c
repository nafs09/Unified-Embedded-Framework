/// @file src/uproto/network/ip.c
/// @brief Validated dispatch for IP-stack and socket providers.
#include "uproto/network/ip.h"

#define UPROTO_IP_HOSTNAME_MAX 253u

static bool uproto_ip_family_valid(uproto_ip_family_t family) {
    return family == UPROTO_IP_FAMILY_IPV4 || family == UPROTO_IP_FAMILY_IPV6;
}

static bool uproto_ip_endpoint_valid(const uproto_ip_endpoint_t* endpoint) {
    return endpoint != NULL && uproto_ip_family_valid(endpoint->family);
}

static bool uproto_ip_capability_for_family(const uproto_ip_t* ip, uproto_ip_family_t family) {
    return family == UPROTO_IP_FAMILY_IPV4 ? (ip->capabilities & UPROTO_IP_CAP_IPV4) != 0u
                                           : (ip->capabilities & UPROTO_IP_CAP_IPV6) != 0u;
}

uetd_status_t uproto_ip_bind(uproto_ip_t* ip, const uproto_ip_ops_t* ops, void* context, uetd_u32_t capabilities) {
    const uetd_u32_t known = UPROTO_IP_CAP_IPV4 | UPROTO_IP_CAP_IPV6 | UPROTO_IP_CAP_TCP | UPROTO_IP_CAP_UDP |
                             UPROTO_IP_CAP_DHCP | UPROTO_IP_CAP_DNS | UPROTO_IP_CAP_SOCKET_OFFLOAD;
    if (ip == NULL || ops == NULL || capabilities == 0u || (capabilities & ~known) != 0u ||
        (capabilities & (UPROTO_IP_CAP_IPV4 | UPROTO_IP_CAP_IPV6)) == 0u) return UETD_INVALID_ARG;
    ip->ops = ops;
    ip->context = context;
    ip->capabilities = capabilities;
    ip->bound = true;
    return UETD_OK;
}

uetd_status_t uproto_ip_dhcp_start(uproto_ip_t* ip, uetd_u8_t interface_id) {
    if (ip == NULL || !ip->bound) return UETD_INVALID_ARG;
    if ((ip->capabilities & UPROTO_IP_CAP_DHCP) == 0u) return UETD_NOT_SUPPORTED;
    if (ip->ops->dhcp_start == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->dhcp_start(ip->context, interface_id);
}

uetd_status_t uproto_ip_dhcp_stop(uproto_ip_t* ip, uetd_u8_t interface_id) {
    if (ip == NULL || !ip->bound) return UETD_INVALID_ARG;
    if ((ip->capabilities & UPROTO_IP_CAP_DHCP) == 0u) return UETD_NOT_SUPPORTED;
    if (ip->ops->dhcp_stop == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->dhcp_stop(ip->context, interface_id);
}

uetd_status_t uproto_ip_configure_interface(uproto_ip_t* ip, uetd_u8_t interface_id,
                                            const uproto_ip_interface_config_t* config) {
    uetd_u8_t maximum_prefix;
    if (ip == NULL || !ip->bound || config == NULL || !uproto_ip_family_valid(config->family)) return UETD_INVALID_ARG;
    if (!uproto_ip_capability_for_family(ip, config->family)) return UETD_NOT_SUPPORTED;
    maximum_prefix = config->family == UPROTO_IP_FAMILY_IPV4 ? 32u : 128u;
    if (config->prefix_length > maximum_prefix) return UETD_INVALID_ARG;
    if (ip->ops->configure_interface == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->configure_interface(ip->context, interface_id, config);
}

uetd_status_t uproto_ip_get_interface_config(uproto_ip_t* ip, uetd_u8_t interface_id,
                                              uproto_ip_interface_config_t* config) {
    uproto_ip_interface_config_t actual;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || config == NULL) return UETD_INVALID_ARG;
    if (ip->ops->get_interface_config == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->get_interface_config(ip->context, interface_id, &actual);
    if (status != UETD_OK) return status;
    if (!uproto_ip_family_valid(actual.family) || !uproto_ip_capability_for_family(ip, actual.family) ||
        actual.prefix_length > (actual.family == UPROTO_IP_FAMILY_IPV4 ? 32u : 128u))
        return UETD_ERROR;
    *config = actual;
    return UETD_OK;
}

uetd_status_t uproto_ip_ipv6_autoconfigure(uproto_ip_t* ip, uetd_u8_t interface_id, bool enable_slaac) {
    if (ip == NULL || !ip->bound) return UETD_INVALID_ARG;
    if ((ip->capabilities & UPROTO_IP_CAP_IPV6) == 0u || ip->ops->ipv6_autoconfigure == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->ipv6_autoconfigure(ip->context, interface_id, enable_slaac);
}

uetd_status_t uproto_ip_dns_resolve(uproto_ip_t* ip, const uetd_u8_t* hostname, uetd_u8_t hostname_length,
                                    uproto_ip_family_t family, uproto_ip_endpoint_t* result) {
    uproto_ip_endpoint_t actual;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || hostname == NULL || hostname_length == 0u || hostname_length > UPROTO_IP_HOSTNAME_MAX ||
        !uproto_ip_family_valid(family) || result == NULL) return UETD_INVALID_ARG;
    if ((ip->capabilities & UPROTO_IP_CAP_DNS) == 0u || !uproto_ip_capability_for_family(ip, family)) return UETD_NOT_SUPPORTED;
    if (ip->ops->dns_resolve == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->dns_resolve(ip->context, hostname, hostname_length, family, &actual);
    if (status != UETD_OK) return status;
    if (!uproto_ip_endpoint_valid(&actual) || actual.family != family) return UETD_ERROR;
    *result = actual;
    return UETD_OK;
}

uetd_status_t uproto_ip_socket_open(uproto_ip_t* ip, uproto_ip_family_t family, uproto_ip_socket_kind_t kind,
                                    uproto_ip_socket_t* socket) {
    uetd_u32_t handle = 0u;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || socket == NULL || !uproto_ip_family_valid(family) ||
        (kind != UPROTO_IP_SOCKET_DATAGRAM && kind != UPROTO_IP_SOCKET_STREAM)) return UETD_INVALID_ARG;
    if (!uproto_ip_capability_for_family(ip, family) ||
        (kind == UPROTO_IP_SOCKET_STREAM && (ip->capabilities & UPROTO_IP_CAP_TCP) == 0u) ||
        (kind == UPROTO_IP_SOCKET_DATAGRAM && (ip->capabilities & UPROTO_IP_CAP_UDP) == 0u)) return UETD_NOT_SUPPORTED;
    if (ip->ops->socket_open == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_open(ip->context, family, kind, &handle);
    if (status != UETD_OK) return status;
    socket->handle = handle;
    socket->kind = kind;
    socket->family = family;
    socket->active = true;
    return UETD_OK;
}

uetd_status_t uproto_ip_socket_connect(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uproto_ip_endpoint_t* endpoint) {
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || !uproto_ip_endpoint_valid(endpoint)) return UETD_INVALID_ARG;
    if (endpoint->family != socket->family) return UETD_INVALID_ARG;
    if (ip->ops->socket_connect == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->socket_connect(ip->context, socket->handle, endpoint);
}

uetd_status_t uproto_ip_socket_bind(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uproto_ip_endpoint_t* endpoint) {
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || !uproto_ip_endpoint_valid(endpoint)) return UETD_INVALID_ARG;
    if (endpoint->family != socket->family) return UETD_INVALID_ARG;
    if (ip->ops->socket_bind == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->socket_bind(ip->context, socket->handle, endpoint);
}

uetd_status_t uproto_ip_socket_listen(uproto_ip_t* ip, uproto_ip_socket_t* socket, uetd_u8_t backlog) {
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || socket->kind != UPROTO_IP_SOCKET_STREAM || backlog == 0u)
        return UETD_INVALID_ARG;
    if (ip->ops->socket_listen == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->socket_listen(ip->context, socket->handle, backlog);
}

uetd_status_t uproto_ip_socket_accept(uproto_ip_t* ip, uproto_ip_socket_t* listener, uproto_ip_socket_t* accepted,
                                      uproto_ip_endpoint_t* peer) {
    uetd_u32_t handle = 0u;
    uproto_ip_endpoint_t actual_peer;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || listener == NULL || !listener->active || listener->kind != UPROTO_IP_SOCKET_STREAM ||
        accepted == NULL || peer == NULL) return UETD_INVALID_ARG;
    if (ip->ops->socket_accept == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_accept(ip->context, listener->handle, &handle, &actual_peer);
    if (status != UETD_OK) return status;
    if (!uproto_ip_endpoint_valid(&actual_peer) || actual_peer.family != listener->family) return UETD_ERROR;
    accepted->handle = handle;
    accepted->kind = UPROTO_IP_SOCKET_STREAM;
    accepted->family = listener->family;
    accepted->active = true;
    *peer = actual_peer;
    return UETD_OK;
}

uetd_status_t uproto_ip_socket_send(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uetd_u8_t* bytes,
                                    uetd_u16_t length, uetd_u16_t* sent) {
    uetd_u16_t actual = 0u;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || bytes == NULL || length == 0u || sent == NULL) return UETD_INVALID_ARG;
    if (ip->ops->socket_send == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_send(ip->context, socket->handle, bytes, length, &actual);
    if (status != UETD_OK) return status;
    if (actual > length) return UETD_OVERFLOW;
    *sent = actual;
    return UETD_OK;
}

uetd_status_t uproto_ip_socket_receive(uproto_ip_t* ip, uproto_ip_socket_t* socket, uetd_u8_t* bytes,
                                       uetd_u16_t capacity, uetd_u16_t* received) {
    uetd_u16_t actual = 0u;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || bytes == NULL || capacity == 0u || received == NULL)
        return UETD_INVALID_ARG;
    if (ip->ops->socket_receive == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_receive(ip->context, socket->handle, bytes, capacity, &actual);
    if (status != UETD_OK) return status;
    if (actual > capacity) return UETD_OVERFLOW;
    *received = actual;
    return actual == 0u ? UETD_NOT_READY : UETD_OK;
}

uetd_status_t uproto_ip_socket_send_to(uproto_ip_t* ip, uproto_ip_socket_t* socket, const uproto_ip_endpoint_t* peer,
                                       const uetd_u8_t* bytes, uetd_u16_t length, uetd_u16_t* sent) {
    uetd_u16_t actual = 0u;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || socket->kind != UPROTO_IP_SOCKET_DATAGRAM ||
        !uproto_ip_endpoint_valid(peer) || bytes == NULL ||
        length == 0u || sent == NULL) return UETD_INVALID_ARG;
    if (peer->family != socket->family) return UETD_INVALID_ARG;
    if (ip->ops->socket_send_to == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_send_to(ip->context, socket->handle, peer, bytes, length, &actual);
    if (status != UETD_OK) return status;
    if (actual > length) return UETD_OVERFLOW;
    *sent = actual;
    return UETD_OK;
}

uetd_status_t uproto_ip_socket_receive_from(uproto_ip_t* ip, uproto_ip_socket_t* socket, uproto_ip_endpoint_t* peer,
                                            uetd_u8_t* bytes, uetd_u16_t capacity, uetd_u16_t* received) {
    uproto_ip_endpoint_t actual_peer;
    uetd_u16_t actual = 0u;
    uetd_status_t status;
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active || socket->kind != UPROTO_IP_SOCKET_DATAGRAM ||
        peer == NULL || bytes == NULL || capacity == 0u || received == NULL)
        return UETD_INVALID_ARG;
    if (ip->ops->socket_receive_from == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_receive_from(ip->context, socket->handle, &actual_peer, bytes, capacity, &actual);
    if (status != UETD_OK) return status;
    if (actual > capacity) return UETD_OVERFLOW;
    if (actual != 0u && (!uproto_ip_endpoint_valid(&actual_peer) || actual_peer.family != socket->family)) return UETD_ERROR;
    if (actual != 0u) *peer = actual_peer;
    *received = actual;
    return actual == 0u ? UETD_NOT_READY : UETD_OK;
}

uetd_status_t uproto_ip_socket_close(uproto_ip_t* ip, uproto_ip_socket_t* socket) {
    uetd_status_t status;
    if (ip == NULL || !ip->bound || socket == NULL || !socket->active) return UETD_INVALID_ARG;
    if (ip->ops->socket_close == NULL) return UETD_NOT_SUPPORTED;
    status = ip->ops->socket_close(ip->context, socket->handle);
    if (status == UETD_OK) socket->active = false;
    return status;
}

uetd_status_t uproto_ip_poll(uproto_ip_t* ip, uetd_u16_t work_budget) {
    if (ip == NULL || !ip->bound || work_budget == 0u) return UETD_INVALID_ARG;
    if (ip->ops->poll == NULL) return UETD_NOT_SUPPORTED;
    return ip->ops->poll(ip->context, work_budget);
}
