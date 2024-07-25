/******************************************************************
* @file: socket.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_socket.h"

#define OBJ_TYPE (OBJ_NONE + SYS + 6)
#define LOCAL_SOCKET_NAME_PATTERN "/var/tmp/cplus_socket_%s"

#define MAX_ADDRESS_SIZE 64

#define HAVE_MSGHDR_MSG_CONTROL

union socketaddr
{
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
    struct sockaddr_un un;
};

struct socket
{
    uint16_t type;
    int32_t socket;
    int32_t domain;
    int32_t style;
    int32_t ip_protocol;
    volatile bool is_connected;
    char address_bufs[MAX_ADDRESS_SIZE];
};

static void * socket_initialize_object(
    bool create_socket
    , int32_t domain
    , int32_t style
    , int32_t ip_protocol)
{
    struct socket * skt = NULL;
    int32_t domain_in = 0, style_in = 0, ip_protocol_in = 0;

    if ((skt = (struct socket *)cplus_malloc(sizeof(struct socket))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(skt);
        skt->type = OBJ_TYPE;
        skt->socket = INVALID_SOCKET;
        skt->domain = domain;
        skt->style = style;
        skt->ip_protocol = ip_protocol;
        skt->is_connected = false;

        switch (skt->domain)
        {
        default:
            errno = EINVAL;
            goto exit;
        case CPLUS_SOCKET_DOMAIN_IPV4:
            {
                domain_in = AF_INET;
            }
            break;
        case CPLUS_SOCKET_DOMAIN_IPV6:
            {
                domain_in = AF_INET6;
            }
            break;
        case CPLUS_SOCKET_DOMAIN_LOCAL:
            {
                domain_in = AF_UNIX;
            }
            break;
        case CPLUS_SOCKET_DOMAIN_UNSPEC:
            {
                domain_in = PF_UNSPEC;
            }
            break;
        }

        switch (skt->style)
        {
        default:
            errno = EINVAL;
            goto exit;
        case CPLUS_SOCKET_STYLE_STREAM:
            {
                style_in = SOCK_STREAM;
            }
            break;
        case CPLUS_SOCKET_STYLE_DGRAM:
            {
                style_in = SOCK_DGRAM;
            }
            break;
        }

        switch (skt->ip_protocol)
        {
        default:
            errno = EINVAL;
            goto exit;
        case CPLUS_SOCKET_IP_PROTOCOL_NONE:
            {
                ip_protocol_in = IPPROTO_IP;
            }
            break;
        case CPLUS_SOCKET_IP_PROTOCOL_TCP:
            {
                ip_protocol_in = IPPROTO_TCP;
            }
            break;
        case CPLUS_SOCKET_IP_PROTOCOL_UDP:
            {
                ip_protocol_in = IPPROTO_UDP;
            }
            break;
        }

        if (true == create_socket)
        {
            if (INVALID_SOCKET == (skt->socket = socket(domain_in
                , style_in
                , ip_protocol_in)))
            {
                goto exit;
            }
        }
    }
    return skt;
exit:
    cplus_socket_delete(skt);
    return NULL;
}

int32_t cplus_socket_delete(cplus_socket obj)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);

    if (INVALID_SOCKET != skt->socket)
    {
        shutdown(skt->socket, SHUT_RDWR);
        close(skt->socket);
        skt->socket = INVALID_SOCKET;
    }

    if (CPLUS_SOCKET_DOMAIN_LOCAL == skt->domain)
    {
        unlink(skt->address_bufs);
    }

    cplus_free(skt);

    return CPLUS_SUCCESS;
}

cplus_socket cplus_socket_new_ex(
    int32_t domain
    , int32_t style
    , int32_t ip_protocol)
{
    CHECK_IN_INTERVAL(
        domain
        , 0
        , (CPLUS_SOCKET_DOMAIN_UNKNOWN - 1)
        , NULL);

    CHECK_IN_INTERVAL(
        style
        , 0
        , (CPLUS_SOCKET_STYLE_UNKNOWN - 1)
        , NULL);

    CHECK_IN_INTERVAL(
        ip_protocol
        , 0
        , (CPLUS_SOCKET_IP_PROTOCOL_UNKNOWN - 1)
        , NULL);

    return socket_initialize_object(true, domain, style, ip_protocol);
}

cplus_socket cplus_socket_new(CPLUS_SOCKET_TYPE type)
{
    cplus_socket skt = NULL;

    switch (type)
    {
    default:
        errno = EINVAL;
        return NULL;

    case CPLUS_SOCKET_TYPE_TCP_IPV4:
        {
            skt = cplus_socket_new_ex(
                CPLUS_SOCKET_DOMAIN_IPV4
                , CPLUS_SOCKET_STYLE_STREAM
                , CPLUS_SOCKET_IP_PROTOCOL_TCP);
        }
        break;
    case CPLUS_SOCKET_TYPE_TCP_IPV6:
        {
            skt = cplus_socket_new_ex(
                CPLUS_SOCKET_DOMAIN_IPV6
                , CPLUS_SOCKET_STYLE_STREAM
                , CPLUS_SOCKET_IP_PROTOCOL_TCP);
        }
        break;
    case CPLUS_SOCKET_TYPE_UDP_IPV4:
        {
            skt = cplus_socket_new_ex(
                CPLUS_SOCKET_DOMAIN_IPV4
                , CPLUS_SOCKET_STYLE_DGRAM
                , CPLUS_SOCKET_IP_PROTOCOL_UDP);
        }
        break;
    case CPLUS_SOCKET_TYPE_UDP_IPV6:
        {
            skt = cplus_socket_new_ex(
                CPLUS_SOCKET_DOMAIN_IPV6
                , CPLUS_SOCKET_STYLE_DGRAM
                , CPLUS_SOCKET_IP_PROTOCOL_UDP);
        }
        break;
    case CPLUS_SOCKET_TYPE_STREAM_LOCAL:
        {
            skt = cplus_socket_new_ex(
                CPLUS_SOCKET_DOMAIN_LOCAL
                , CPLUS_SOCKET_STYLE_STREAM
                , CPLUS_SOCKET_IP_PROTOCOL_NONE);
        }
        break;
    case CPLUS_SOCKET_TYPE_DGRAM_LOCAL:
        {
            skt = cplus_socket_new_ex(
                CPLUS_SOCKET_DOMAIN_LOCAL
                , CPLUS_SOCKET_STYLE_DGRAM
                , CPLUS_SOCKET_IP_PROTOCOL_NONE);
        }
        break;
    case CPLUS_SOCKET_TYPE_DUEL_STACK:
        errno = ENOTSUP;
        break;
    }

    return skt;
}

int32_t cplus_socket_shutdown(cplus_socket obj)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);
    CHECK_IF(false == skt->is_connected, CPLUS_SUCCESS);
    CHECK_IF(CPLUS_SOCKET_STYLE_STREAM != skt->style, CPLUS_SUCCESS);

    return shutdown(skt->socket, SHUT_RDWR);
}

int32_t cplus_socket_shutdown_ex(
    cplus_socket obj
    , CPLUS_SOCKET_SHUTDOWN_MODE mode)
{
    struct socket * skt = (struct socket *)(obj);
    int mode_t = 0;
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);
    CHECK_IF(false == skt->is_connected, CPLUS_SUCCESS);
    CHECK_IF(CPLUS_SOCKET_STYLE_STREAM != skt->style, CPLUS_SUCCESS);

    switch (mode)
    {
    default:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_SOCKET_SHUTDOWN_MODE_READ:
        {
            mode_t = SHUT_RD;
        }
        break;
    case CPLUS_SOCKET_SHUTDOWN_MODE_WRITE:
        {
            mode_t = SHUT_WR;
        }
        break;
    case CPLUS_SOCKET_SHUTDOWN_MODE_BOTH:
        {
            mode_t = SHUT_RDWR;
        }
        break;
    }

    return shutdown(skt->socket, mode_t);
}

int32_t cplus_socket_close(cplus_socket obj)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    if (0 == close(skt->socket))
    {
        skt->socket = INVALID_SOCKET;
        return CPLUS_SUCCESS;
    }
    return CPLUS_FAIL;
}

static int32_t to_socket_addr(
    int32_t domin
    , const char * from_addr
    , int32_t from_port
    , void * to_addr
    , socklen_t * to_addr_size)
{
    union socketaddr * addr_t = (union socketaddr *)(to_addr);
    CHECK_NOT_NULL(to_addr, CPLUS_FAIL);

    if (NULL == from_addr)
    {
        to_addr = NULL;
        (* to_addr_size) = 0;
    }
    else
    {
        switch(domin)
        {
        default:
        case CPLUS_SOCKET_DOMAIN_UNKNOWN:
            {
                errno = ENOTSUP;
            }
            return CPLUS_FAIL;
        case CPLUS_SOCKET_DOMAIN_IPV4:
            {
                if (1 != inet_pton(
                    AF_INET
                    , from_addr
                    , &(addr_t->in.sin_addr.s_addr)))
                {
                    errno = EINVAL;
                    return CPLUS_FAIL;
                }
                addr_t->in.sin_family = AF_INET;
                addr_t->in.sin_port = htons(from_port);
                (* to_addr_size) = sizeof(addr_t->in);
            }
            break;
        case CPLUS_SOCKET_DOMAIN_IPV6:
            {
                if (1 != inet_pton(
                    AF_INET6
                    , from_addr
                    , &(addr_t->in6.sin6_addr.s6_addr)))
                {
                    errno = EINVAL;
                    return CPLUS_FAIL;
                }
                addr_t->in6.sin6_family = AF_INET6;
                addr_t->in6.sin6_port = htons(from_port);
                addr_t->in6.sin6_flowinfo = 0;
                addr_t->in6.sin6_scope_id = 0;
                (* to_addr_size) = sizeof(addr_t->in6);
            }
            break;
        case CPLUS_SOCKET_DOMAIN_LOCAL:
            {
                addr_t->un.sun_family = AF_UNIX;
                cplus_str_printf(
                    addr_t->un.sun_path
                    , sizeof(addr_t->un.sun_path) - 1
                    , LOCAL_SOCKET_NAME_PATTERN
                    , from_addr);
                (* to_addr_size) = sizeof(addr_t->un);
            }
            break;
        }
    }

    return CPLUS_SUCCESS;
}

static int32_t ready_to_recv(int32_t sock_fd, uint32_t timeout)
{
    int32_t res = 0;
    fd_set read_fds;
	struct timeval select_timeout;

    if (CPLUS_INFINITE_TIMEOUT != timeout)
    {
        timeout = CPLUS_MAX(1U, timeout);
        select_timeout.tv_sec = timeout / 1000;
		select_timeout.tv_usec = (timeout % 1000) * 1000;

        FD_ZERO(&read_fds);
		FD_SET(sock_fd, &read_fds);

        if (0 >= (res = select(sock_fd + 1, &read_fds, NULL, NULL, &select_timeout)))
        {
            errno = (0 == res)? ETIMEDOUT: errno;
            return CPLUS_FAIL;
        }
    }
    return CPLUS_SUCCESS;
}

static int32_t ready_to_send(int32_t sock_fd, uint32_t timeout)
{
    int32_t res = 0;
    fd_set send_fds;
	struct timeval select_timeout;

    if (CPLUS_INFINITE_TIMEOUT != timeout)
    {
        timeout = CPLUS_MAX(1U, timeout);
        select_timeout.tv_sec = timeout / 1000;
		select_timeout.tv_usec = (timeout % 1000) * 1000;

        FD_ZERO(&send_fds);
		FD_SET(sock_fd, &send_fds);

        if (0 >= (res = select(sock_fd + 1, NULL, &send_fds, NULL, &select_timeout)))
        {
            errno = (0 == res)? ETIMEDOUT: errno;
            return CPLUS_FAIL;
        }
    }
    return CPLUS_SUCCESS;
}

bool cplus_socket_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

int32_t cplus_socket_bind(
    cplus_socket obj
    , const char * addr
    , int32_t port)
{
    int32_t res = CPLUS_FAIL;
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    socklen_t addr_size = 0;
    union socketaddr addr_t = {0};

    if (CPLUS_SUCCESS == to_socket_addr(skt->domain, addr, port, &addr_t, &addr_size))
    {
        if (CPLUS_SOCKET_DOMAIN_LOCAL == skt->domain)
        {
            cplus_str_printf(skt->address_bufs, MAX_ADDRESS_SIZE - 1, LOCAL_SOCKET_NAME_PATTERN, addr);
            unlink(skt->address_bufs);
        }
        else
        {
            cplus_str_printf(skt->address_bufs, MAX_ADDRESS_SIZE - 1, "%s", addr);
        }

        res = bind(skt->socket, (struct sockaddr *)(&addr_t), addr_size);
    }

    return res;
}

static int32_t socket_connect(
    int32_t socket
    , const struct sockaddr *address
    , socklen_t address_len)
{
    return connect(socket, address, address_len);
}

int32_t cplus_socket_connect(
    cplus_socket obj
    , const char * addr
    , int32_t port)
{
    int32_t res = CPLUS_FAIL;
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    socklen_t addr_size = 0;
    union socketaddr addr_t = {0};

    if (CPLUS_SUCCESS == to_socket_addr(skt->domain, addr, port, &addr_t, &addr_size))
    {
        res = socket_connect(skt->socket, (struct sockaddr *)(&addr_t), addr_size);

        if (0 == res)
        {
            skt->is_connected = true;
        }
    }

    return res;
}

int32_t cplus_socket_listen(cplus_socket obj, int32_t max_conn)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    return listen(skt->socket, CPLUS_MIN(max_conn, SOMAXCONN));
}

cplus_socket cplus_socket_accept(cplus_socket obj, uint32_t timeout)
{
    struct socket * skt = (struct socket *)(obj);
    struct socket * new_skt = NULL;
    struct sockaddr connected_addr = {0};
    socklen_t connected_addr_len = sizeof(struct sockaddr);

    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, NULL);

    if (CPLUS_SUCCESS != ready_to_recv(skt->socket, timeout))
    {
        return NULL;
    }

    int client_socket = accept(skt->socket
        , &connected_addr
        , &connected_addr_len);

    if (INVALID_SOCKET == client_socket)
    {
        return NULL;
    }

    skt->is_connected = true;

    new_skt = (struct socket *)socket_initialize_object(
        false, skt->domain, skt->style, skt->ip_protocol);

    new_skt->socket = client_socket;

    return new_skt;
}

int32_t cplus_socket_recvfrom(
    cplus_socket obj
    , void * data_bufs
    , int32_t data_len
    , char * from_addr
    , int32_t from_addr_size
    , int32_t * from_port
    , uint32_t timeout)
{
    int32_t res = 0;
    struct socket * skt = (struct socket *)(obj);
    char local_name[128] = {0};
    union socketaddr addr = {0};
    socklen_t addr_size = 0;
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);
    CHECK_NOT_NULL(data_bufs, CPLUS_FAIL);
    CHECK_GT_ZERO(data_len, CPLUS_FAIL);

    if (CPLUS_SUCCESS != ready_to_recv(skt->socket, timeout))
    {
        return CPLUS_FAIL;
    }

    if (!(NULL != from_addr AND 0 < from_addr_size AND NULL != from_port))
    {
        res = recv(skt->socket, data_bufs, data_len, 0);
    }
    else
    {
        switch (skt->domain)
        {
        default:
            break;
        case CPLUS_SOCKET_DOMAIN_IPV4:
            {
                addr_size = sizeof(struct sockaddr_in);
            }
            break;
        case CPLUS_SOCKET_DOMAIN_IPV6:
            {
                addr_size = sizeof(struct sockaddr_in6);
            }
            break;
        case CPLUS_SOCKET_DOMAIN_LOCAL:
            {
                addr_size = sizeof(struct sockaddr_un);
            }
            break;
        }

        if (0 < (res = recvfrom(skt->socket
            , data_bufs
            , data_len
            , 0
            , (struct sockaddr *)(&(addr))
            , &addr_size)))
        {
            switch (skt->domain)
            {
            default:
                break;
            case CPLUS_SOCKET_DOMAIN_IPV4:
                {
                    inet_ntop(AF_INET, &(addr.in.sin_addr), from_addr, from_addr_size - 1);
                    (* from_port) = ntohs(addr.in.sin_port);
                }
                break;
            case CPLUS_SOCKET_DOMAIN_IPV6:
                {
                    inet_ntop(AF_INET6, &(addr.in6.sin6_addr), from_addr, from_addr_size - 1);
                    (* from_port) = ntohs(addr.in6.sin6_port);
                }
                break;
            case CPLUS_SOCKET_DOMAIN_LOCAL:
                {
                    if (1 == sscanf(addr.un.sun_path, LOCAL_SOCKET_NAME_PATTERN, local_name))
                    {
                        cplus_str_printf(from_addr, from_addr_size - 1, "%s", local_name);
                    }
                    (* from_port) = 0;
                }
                break;
            }
        }
    }
    return res;
}

int32_t cplus_socket_recv(
    cplus_socket obj
    , void * data_bufs
    , int32_t data_len
    , uint32_t timeout)
{
    return cplus_socket_recvfrom(obj, data_bufs, data_len, NULL, 0, NULL, timeout);
}

int32_t cplus_socket_sendto(
    cplus_socket obj
    , void * data_bufs
    , int32_t data_len
    , const char * addr
    , int32_t port)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);
    CHECK_NOT_NULL(data_bufs, CPLUS_FAIL);
    CHECK_GT_ZERO(data_len, CPLUS_FAIL);

    socklen_t addr_size = 0;
    union socketaddr addr_t = {0};

    if (CPLUS_SUCCESS != to_socket_addr(skt->domain, addr, port, &addr_t, &addr_size))
    {
        return CPLUS_FAIL;
    }

    if (CPLUS_SUCCESS != ready_to_send(skt->socket, CPLUS_INFINITE_TIMEOUT))
    {
        return CPLUS_FAIL;
    }

    return sendto(skt->socket
        , data_bufs
        , data_len
        , MSG_NOSIGNAL
        , (struct sockaddr *)(&(addr_t))
        , addr_size);
}

int32_t cplus_socket_send(
    cplus_socket obj
    , void * data_bufs
    , int32_t data_len)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);
    CHECK_NOT_NULL(data_bufs, CPLUS_FAIL);
    CHECK_GT_ZERO(data_len, CPLUS_FAIL);

    if (CPLUS_SUCCESS != ready_to_send(skt->socket, CPLUS_INFINITE_TIMEOUT))
    {
        return CPLUS_FAIL;
    }

    return send(skt->socket, data_bufs, data_len, MSG_NOSIGNAL);
}

static int32_t socket_sendmsg(cplus_socket obj, struct msghdr * msg)
{
    struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    if (CPLUS_SUCCESS != ready_to_send(skt->socket, CPLUS_INFINITE_TIMEOUT))
    {
        return CPLUS_FAIL;
    }

    return sendmsg(skt->socket, msg, MSG_NOSIGNAL);
}

static int32_t socket_recvmsg(cplus_socket obj
    , char * from_addr
    , int32_t from_addr_size
    , struct msghdr * msg
    , uint32_t timeout)
{
    int32_t res = CPLUS_SUCCESS;
    struct socket * skt = (struct socket *)(obj);
    union socketaddr * addr = NULL;
    char local_name[128] = {0};
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    if (CPLUS_SUCCESS != ready_to_recv(skt->socket, timeout))
    {
        return CPLUS_FAIL;
    }

    if (0 < (res = recvmsg(skt->socket, msg, 0)))
    {
        if ((addr = (union socketaddr *)(msg->msg_name)))
        {
            switch (skt->domain)
            {
            default:
                break;
            case CPLUS_SOCKET_DOMAIN_IPV4:
                {
                    inet_ntop(AF_INET, &(addr->in.sin_addr), from_addr, from_addr_size - 1);
                }
                break;
            case CPLUS_SOCKET_DOMAIN_IPV6:
                {
                    inet_ntop(AF_INET6, &(addr->in6.sin6_addr), from_addr, from_addr_size - 1);
                }
                break;
            case CPLUS_SOCKET_DOMAIN_LOCAL:
                {
                    if (1 == sscanf(addr->un.sun_path, LOCAL_SOCKET_NAME_PATTERN, local_name))
                    {
                        cplus_str_printf(from_addr, from_addr_size - 1, "%s", local_name);
                    }
                }
                break;
            }
        }
    }
    return res;
}

int32_t cplus_socket_setopt_reuse_addr(cplus_socket obj, bool enable_reuse_addr)
{
	struct socket * skt = (struct socket *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF(INVALID_SOCKET == skt->socket, CPLUS_FAIL);

    int value = enable_reuse_addr;

	return (0 == setsockopt(
        skt->socket
        , SOL_SOCKET
        , SO_REUSEADDR
        , (char *)&value
        , sizeof(value)))? CPLUS_SUCCESS: CPLUS_FAIL;
}

int32_t cplus_socket_send_fd(cplus_socket obj
    , int32_t sendfd
    , const char * addr
    , int32_t port)
{
    struct socket * skt = (struct socket *)(obj);
    uint8_t data = 0;
    struct msghdr msg = {0};
    struct iovec iov[1] = {0};
    CHECK_OBJECT_TYPE(obj);

    socklen_t addr_size = 0;
    union socketaddr addr_t = {0};
    if (CPLUS_SUCCESS != to_socket_addr(skt->domain, addr, port, &addr_t, &addr_size))
    {
        return CPLUS_FAIL;
    }

    #ifdef HAVE_MSGHDR_MSG_CONTROL
    {
        struct cmsghdr * cmptr = NULL;
        union
        {
            struct cmsghdr cm;
            uint8_t control[CMSG_SPACE(sizeof(uint32_t))];
        } control_un;

        msg.msg_control = control_un.control;
        msg.msg_controllen = sizeof(control_un.control);

        cmptr = CMSG_FIRSTHDR(&msg);
        cmptr->cmsg_len = CMSG_LEN(sizeof(int));
        cmptr->cmsg_level = SOL_SOCKET;
        cmptr->cmsg_type = SCM_RIGHTS;
        *((int32_t *)CMSG_DATA(cmptr)) = sendfd;

    }
    #else
    {
        msg.msg_accrights = (caddr_t)&sendfd;
        msg.msg_accrightslen = sizeof(int32_t);
    }
    #endif

    msg.msg_name = (addr)? &addr_t: NULL;
    msg.msg_namelen = (addr)? addr_size: 0;
    iov[0].iov_base = &data;
    iov[0].iov_len = sizeof(data);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    return ((int32_t)(msg.msg_iovlen) == socket_sendmsg(skt, &msg))? CPLUS_SUCCESS: CPLUS_FAIL;
}

#if 0 /* have not completed yat */
int32_t cplus_socket_recv_fd(
    cplus_socket obj
    , int32_t * recvfd
    , char * from_addr
    , int32_t from_addr_size
    , uint32_t timeout)
#else
int32_t cplus_socket_recv_fd(
    cplus_socket obj
    , int32_t * recvfd
    , uint32_t timeout)
#endif
{
    int32_t res = CPLUS_FAIL, sock_fd = INVALID_SOCKET;
    struct socket * skt = (struct socket *)(obj);
    uint8_t data = 0;
    struct msghdr msg = {0};
    struct iovec iov[1] = {0};
    char from_addr[64] = {0};
    CHECK_OBJECT_TYPE(obj);

    #ifdef HAVE_MSGHDR_MSG_CONTROL
    {
        union
        {
            struct cmsghdr cm;
            uint8_t control[CMSG_SPACE(sizeof(uint32_t))];
        } control_un;

        msg.msg_control = control_un.control;
        msg.msg_controllen = sizeof(control_un.control);
    }
    #else
    {
        msg.msg_accrights = (caddr_t)(&sock_fd);
        msg.msg_accrightslen = sizeof(int32_t);
    }
    #endif

    iov[0].iov_base = &data;
    iov[0].iov_len = sizeof(data);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if (0 < (res = socket_recvmsg(skt, (char *)(from_addr), sizeof(from_addr), &msg, timeout)))
    {

        #ifdef HAVE_MSGHDR_MSG_CONTROL
        {
            struct cmsghdr * cmptr = NULL;

            if ((cmptr = CMSG_FIRSTHDR(&msg)))
            {
                if (CMSG_LEN(sizeof(int32_t)) != cmptr->cmsg_len
                    || SOL_SOCKET != cmptr->cmsg_level
                    || SCM_RIGHTS != cmptr->cmsg_type)
                {
                    return CPLUS_FAIL;
                }
                sock_fd = *((int32_t *)CMSG_DATA(cmptr));
            }
        }
        #else
        {
            if (msg.msg_accrightslen != sizeof(int32_t))
            {
                return CPLUS_FAIL;
            }
        }
        #endif

        if (recvfd) { (* recvfd) = sock_fd; }

        res = (res == (int32_t)(msg.msg_iovlen))? CPLUS_SUCCESS: CPLUS_FAIL;
    }
    return res;
}

#ifdef __CPLUS_UNITTEST__
#include "cplus_systime.h"
#include "cplus_file.h"

#define SERVER_NAME "unittest_server"
#define CLIENT_NAME "unittest_client"
#define SERVER_PORT 12345
#define CLIENT_PORT 54321
#define TEST_FILE   "/var/tmp/test"
#define TEST_STR    "Hello World"


CPLUS_UNIT_TEST(CPLUS_SOCKET_TYPE_DGRAM_LOCAL, functionity)
{
    cplus_socket skt_server = NULL, skt_client = NULL;
    char data_bufs[100] = {0}, from_addr[64] = {0};
    int32_t from_port = 0;

    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, SERVER_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_client, CLIENT_NAME, CLIENT_PORT));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_sendto(skt_client
        , (void *)("AaBbCcDd"), strlen("AaBbCcDd") + 1, SERVER_NAME, SERVER_PORT));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_recvfrom(skt_server
        , data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(CLIENT_NAME, from_addr));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "AaBbCcDd"));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_sendto(skt_client, (void *)("Hello World"), strlen("Hello World") + 1, SERVER_NAME, SERVER_PORT));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recvfrom(skt_server, data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(CLIENT_NAME, from_addr));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_SOCKET_TYPE_STREAM_LOCAL, functionity)
{
    cplus_socket skt_server = NULL, skt_client = NULL, skt_remote = NULL;
    char data_bufs[100] = {0};

    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_remote = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_remote, SERVER_NAME, 0));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_send(skt_remote, (void *)("Hello World"), strlen("Hello World") + 1));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recv(skt_client, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
    UNITTEST_EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_send(skt_client, (void *)("0123456789"), strlen("0123456789") + 1));
    UNITTEST_EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_recv(skt_remote, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "0123456789"));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_SOCKET_TYPE_UDP_IPV4, functionity)
{
    cplus_socket skt_server = NULL, skt_client = NULL;
    char data_bufs[100] = {0}, from_addr[64] = {0};
    int32_t from_port = 0;

    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_UDP_IPV4)));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_UDP_IPV4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, "127.0.0.1", SERVER_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_client, "127.0.0.1", CLIENT_PORT));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_sendto(skt_client
        , (void *)("Hello World"), strlen("Hello World") + 1, "127.0.0.1", SERVER_PORT));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recvfrom(skt_server
        , data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
    UNITTEST_EXPECT_EQ(0, strcmp(from_addr, "127.0.0.1"));
    UNITTEST_EXPECT_EQ(true, (from_port == CLIENT_PORT));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_sendto(skt_server, (void *)("AaBbCcDd"), strlen("AaBbCcDd") + 1, "127.0.0.1", CLIENT_PORT));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_recvfrom(skt_client, data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "AaBbCcDd"));
    UNITTEST_EXPECT_EQ(0, strcmp(from_addr, "127.0.0.1"));
    UNITTEST_EXPECT_EQ(true, (from_port == SERVER_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_SOCKET_TYPE_TCP_IPV4, functionity)
{
    cplus_socket skt_server = NULL, skt_client = NULL, skt_remote = NULL;
    char data_bufs[100] = {0};

    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_TCP_IPV4)));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_remote = cplus_socket_new(CPLUS_SOCKET_TYPE_TCP_IPV4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_setopt_reuse_addr(skt_server, true));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, "127.0.0.1", SERVER_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_remote, "127.0.0.1", CLIENT_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_remote, "127.0.0.1", SERVER_PORT));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_send(skt_remote, (void *)("Hello World"), strlen("Hello World") + 1));
    UNITTEST_EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recv(skt_client, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
    UNITTEST_EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_send(skt_client, (void *)("0123456789"), strlen("0123456789") + 1));
    UNITTEST_EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_recv(skt_remote, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(0, strcmp(data_bufs, "0123456789"));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_socket_send_fd, CPLUS_SOCKET_TYPE_STREAM_LOCAL)
{
    cplus_socket skt_server = NULL, skt_client = NULL, skt_remote = NULL;
    cplus_file test_file = NULL;
    int32_t recv_fd = INVALID_FD, send_fd = INVALID_FD;
    char rr[32] = {0};

    UNITTEST_EXPECT_EQ(true, (NULL != (test_file = cplus_file_new((char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR))));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_write(test_file, strlen(TEST_STR), (void *)(TEST_STR)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_reset_pos(test_file));
    UNITTEST_EXPECT_EQ(true, (INVALID_FD != (send_fd = cplus_file_get_fd(test_file))));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_client, SERVER_NAME, 0));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_remote = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_send_fd(skt_remote, send_fd, NULL, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_recv_fd(skt_client, &recv_fd, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), read(recv_fd, rr, sizeof(rr)));
    UNITTEST_EXPECT_EQ(true, (0 == strcmp(TEST_STR, rr)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(test_file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_socket_send_fd, SERVER)
{
    cplus_socket skt_server = NULL, skt_remote = NULL;
    cplus_file test_file = NULL;
    int32_t send_fd = INVALID_FD;

    UNITTEST_EXPECT_EQ(true, (NULL != (test_file = cplus_file_new((char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR))));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_write(test_file, strlen(TEST_STR), (void *)(TEST_STR)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_reset_pos(test_file));
    UNITTEST_EXPECT_EQ(true, (INVALID_FD != (send_fd = cplus_file_get_fd(test_file))));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_remote = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_send_fd(skt_remote, send_fd, NULL, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(test_file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_socket_recv_fd, CLIENT)
{
    cplus_socket skt_client = NULL;
    int32_t recv_fd = INVALID_FD;
    char rr[32] = {0};

    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_client, SERVER_NAME, 0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_recv_fd(skt_client, &recv_fd, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), read(recv_fd, rr, sizeof(rr)));
    UNITTEST_EXPECT_EQ(true, (0 == strcmp(TEST_STR, rr)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_socket_send_fd, CPLUS_SOCKET_TYPE_DGRAM_LOCAL)
{
    cplus_socket skt_server = NULL, skt_client = NULL;
    cplus_file test_file = NULL;
    char rr[32] = {0};
    int32_t recv_fd = INVALID_FD, send_fd = INVALID_FD;

    UNITTEST_EXPECT_EQ(true, (NULL != (test_file = cplus_file_new((char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR))));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), cplus_file_write(test_file, strlen(TEST_STR), (void *)(TEST_STR)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_reset_pos(test_file));
    UNITTEST_EXPECT_EQ(true, (INVALID_FD != (send_fd = cplus_file_get_fd(test_file))));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
    UNITTEST_EXPECT_EQ(true, NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, SERVER_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_client, CLIENT_NAME, CLIENT_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_send_fd(skt_server, send_fd, CLIENT_NAME, CLIENT_PORT));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_recv_fd(skt_client, &recv_fd, CPLUS_INFINITE_TIMEOUT));
    UNITTEST_EXPECT_EQ(strlen(TEST_STR), read(recv_fd, rr, sizeof(rr)));
    UNITTEST_EXPECT_EQ(true, (0 == strcmp(TEST_STR, rr)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(test_file));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());

}

void unittest_socket(void)
{
    UNITTEST_ADD_TESTCASE(CPLUS_SOCKET_TYPE_DGRAM_LOCAL, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_SOCKET_TYPE_STREAM_LOCAL, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_SOCKET_TYPE_UDP_IPV4, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_SOCKET_TYPE_TCP_IPV4, functionity);
    UNITTEST_ADD_TESTCASE(cplus_socket_send_fd, CPLUS_SOCKET_TYPE_STREAM_LOCAL);
    UNITTEST_ADD_TESTCASE(cplus_socket_send_fd, CPLUS_SOCKET_TYPE_DGRAM_LOCAL);
}

void unittest_socket_server(void)
{
    UNITTEST_ADD_TESTCASE(cplus_socket_send_fd, SERVER);
}

void unittest_socket_client(void)
{
    UNITTEST_ADD_TESTCASE(cplus_socket_recv_fd, CLIENT);
}

#endif // __CPLUS_UNITTEST__