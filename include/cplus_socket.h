#ifndef __CPLUS_SOCKET_H__
#define __CPLUS_SOCKET_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cplus_socket_shutdown_mode
{
    CPLUS_SOCKET_SHUTDOWN_MODE_READ,
    CPLUS_SOCKET_SHUTDOWN_MODE_WRITE,
    CPLUS_SOCKET_SHUTDOWN_MODE_BOTH,
    CPLUS_SOCKET_SHUTDOWN_MODE_UNKNOWN,
}CPLUS_SOCKET_SHUTDOWN_MODE;

typedef enum cplus_socket_domain
{
    CPLUS_SOCKET_DOMAIN_IPV4,
    CPLUS_SOCKET_DOMAIN_IPV6,
    CPLUS_SOCKET_DOMAIN_LOCAL,
    CPLUS_SOCKET_DOMAIN_UNSPEC,
    CPLUS_SOCKET_DOMAIN_UNKNOWN,
}CPLUS_SOCKET_DOMAIN;

typedef enum cplus_socket_style
{
    CPLUS_SOCKET_STYLE_STREAM,
    CPLUS_SOCKET_STYLE_DGRAM,
    CPLUS_SOCKET_STYLE_UNKNOWN,
}CPLUS_SOCKET_STYLE;

typedef enum cplus_socket_ip_protocol
{
    CPLUS_SOCKET_IP_PROTOCOL_NONE,
    CPLUS_SOCKET_IP_PROTOCOL_TCP,
    CPLUS_SOCKET_IP_PROTOCOL_UDP,
    CPLUS_SOCKET_IP_PROTOCOL_UNKNOWN,
}CPLUS_SOCKET_IP_PROTOCOL;

typedef enum cplus_socket_type
{
    CPLUS_SOCKET_TYPE_TCP_IPV4,
    CPLUS_SOCKET_TYPE_TCP_IPV6,
    CPLUS_SOCKET_TYPE_UDP_IPV4,
    CPLUS_SOCKET_TYPE_UDP_IPV6,
    CPLUS_SOCKET_TYPE_STREAM_LOCAL,
    CPLUS_SOCKET_TYPE_DGRAM_LOCAL,
    CPLUS_SOCKET_TYPE_DUEL_STACK,
    CPLUS_SOCKET_TYPE_UNKNOWN,
}CPLUS_SOCKET_TYPE;

cplus_socket cplus_socket_new(CPLUS_SOCKET_TYPE type);
cplus_socket cplus_socket_new_ex(int32_t domain, int32_t style, int32_t ip_protocol);
int32_t cplus_socket_delete(cplus_socket obj);
bool cplus_socket_check(cplus_object obj);
int32_t cplus_socket_shutdown(cplus_socket obj);
int32_t cplus_socket_shutdown_ex(cplus_socket obj, CPLUS_SOCKET_SHUTDOWN_MODE mode);
int32_t cplus_socket_close(cplus_socket obj);
int32_t cplus_socket_bind(cplus_socket obj, const char * addr, int32_t port);
int32_t cplus_socket_connect(cplus_socket obj, const char * addr, int32_t port);
int32_t cplus_socket_listen(cplus_socket obj, int32_t max_conn);
cplus_socket cplus_socket_accept(cplus_socket obj, uint32_t timeout);
int32_t cplus_socket_recvfrom(
    cplus_socket obj, void * data_bufs, int32_t data_len, char * from_addr, int32_t from_addr_size, int32_t * from_port, uint32_t timeout);
int32_t cplus_socket_recv(cplus_socket obj, void * data_bufs, int32_t data_len, uint32_t timeout);
int32_t cplus_socket_sendto(cplus_socket obj, void * data_bufs, int32_t data_len, const char * addr, int32_t port);
int32_t cplus_socket_send(cplus_socket obj, void * data_bufs, int32_t data_len);
int32_t cplus_socket_setopt_reuse_addr(cplus_socket obj, bool enable_reuse_addr);
int32_t cplus_socket_recv_fd(cplus_socket obj, int32_t * recvfd, uint32_t timeout);
int32_t cplus_socket_send_fd(cplus_socket obj, int32_t sendfd, const char * addr, int32_t port);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_SOCKET_H__