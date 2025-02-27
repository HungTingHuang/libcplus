/******************************************************************
* @file: event_server.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <sys/eventfd.h>
#include <poll.h>
#include "common.h"
#include "cplus.h"
#include "cplus_socket.h"
#include "cplus_event_server.h"

#define OBJ_TYPE_SERVER (OBJ_NONE + SYS + 9)
#define OBJ_TYPE_CLIENT (OBJ_NONE + SYS + 10)

#define DURATION_FOR_POLL_PROC 100U
#define TIMEOUT_FOR_STOP_EVENT_POLL 500U
#define DEFAULT_POLL_EVENT_COUNT 10
#define DEFAULT_POLL_EVENT_TIMEOUT 500U
#define DEFAULT_EVENT_SERVER_NAME "fd_pass"
#define DEFAULT_LISTEN_COUNT 10
#define TIMEOUT_FOR_STOP_ACCEPT_TASK 500U
#define DURATION_FOR_ACCEPT_TASK 100U
#define TIMEOUT_FOR_ACCEPT_TASK 500U
#define TIMEOUT_FOR_RECV_PASS_FD_ACK 5000U
#define CHECK_RECV_FD "RECV_FD"
#define TIMEOUT_FOR_RECV_FD 3000U

typedef struct event_server
{
    uint16_t type;
    int32_t efd;
    bool enable_ipc;
    cplus_socket skt_server;
    cplus_task accept_proc;
    uint32_t init_val;
    enum cplus_event_server_flag flag;
    bool start;
    cplus_task poll_proc;
    void * cb_param;
    CPLUS_EVENT_SERVER_CB_ON_ERROR on_error;
    CPLUS_EVENT_SERVER_CB_ON_TIMEOUT on_timeout;
    CPLUS_EVENT_SERVER_CB_ON_READ on_read;
} * EVENT_SERVER, EVENT_SERVER_T;

typedef struct event_client
{
    uint16_t type;
    int32_t server_efd;
    cplus_socket skt_client;
} * EVENT_CLIENT, EVENT_CLIENT_T;

static int32_t convert_efd_flag(CPLUS_EVENT_SERVER_FLAG flag)
{
    int32_t flag_value = 0;
    if (flag == CPLUS_EVENT_SERVER_FLAG_NONE)
    {
        return flag_value;
    }
    if (flag & CPLUS_EVENT_SERVER_FLAG_CLOEXEC)
    {
        flag_value |= EFD_CLOEXEC;
    }
    if (flag & CPLUS_EVENT_SERVER_FLAG_NONBLOCK)
    {
        flag_value |= EFD_NONBLOCK;
    }
    if (flag & CPLUS_EVENT_SERVER_FLAG_SYNC)
    {
        flag_value |= EFD_SEMAPHORE;
    }
    return flag_value;
}

int32_t cplus_event_server_delete(cplus_event_server obj)
{
    struct event_server * server = (struct event_server *)(obj);
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_SERVER);

    if (server->accept_proc)
    {
        cplus_task_stop(server->accept_proc, TIMEOUT_FOR_STOP_ACCEPT_TASK);
    }

    if (server->skt_server)
    {
        cplus_socket_delete(server->skt_server);
    }

    if (server->poll_proc)
    {
        cplus_task_stop(server->poll_proc, TIMEOUT_FOR_STOP_EVENT_POLL);
    }

    if (INVALID_FD != server->efd)
    {
        close(server->efd);
    }

    cplus_free(server);
    return CPLUS_SUCCESS;
}

int32_t cplus_event_client_delete(cplus_event_client obj)
{
    struct event_client * client = (struct event_client *)(obj);
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_CLIENT);

    if (client->skt_client)
    {
        cplus_socket_delete(client->skt_client);
    }

    cplus_free(client);
    return CPLUS_SUCCESS;
}

static void event_poll_proc(void * param1, void * param2)
{
    struct event_server * server = (struct event_server *)(param1);
    struct pollfd pollfds[1] = {0};
    uint64_t read_value = 0;
    int32_t value = 0;
    UNUSED_PARAM(param2);

    pollfds[0].fd = server->efd;
    pollfds[0].events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
    pollfds[0].revents = 0;
    switch(poll(pollfds, 1, CPLUS_INFINITE_TIMEOUT))
    {
    case -1: /* error */
        {
            if (server->on_error)
            {
                server->on_error(pollfds[0].fd, errno, server->cb_param);
            }
        }
        break;
    case 0: /* tiemout */
        {
            if (server->on_timeout)
            {
                server->on_timeout(pollfds[0].fd, server->cb_param);
            }
        }
        break;
    default:
        {
            if ((pollfds[0].revents & POLLIN) && server->on_read)
            {
                if (sizeof(uint64_t) == read(pollfds[0].fd, &read_value, sizeof(uint64_t)))
                {
                    value = (int32_t)(read_value);
                    server->on_read(pollfds[0].fd, value, server->cb_param);
                    value = 0;
                }
            }
            if (pollfds[0].revents & POLLPRI)
            {
                ;
            }
            if (pollfds[0].revents & POLLERR)
            {
                ;
            }
            if (pollfds[0].revents & POLLHUP)
            {
                ;
            }
            if (pollfds[0].revents & POLLNVAL)
            {
                ;
            }
        }
        break;
    }
    return;
}

static void accept_proc(void * param1, void * param2)
{
    struct event_server * server = (struct event_server *)(param1);
    cplus_socket sock = CPLUS_NULL;
    uint8_t recv_bufs[32] = {0};
    UNUSED_PARAM(param2);

    if ((sock = cplus_socket_accept(server->skt_server, CPLUS_INFINITE_TIMEOUT)))
    {
        cplus_socket_send_fd(sock, server->efd, CPLUS_NULL, 0);
        if (strlen(CHECK_RECV_FD) == cplus_socket_recv(sock
            , recv_bufs
            , sizeof(recv_bufs)
            , TIMEOUT_FOR_RECV_PASS_FD_ACK))
        {
            if (0 == strcmp((char *)(recv_bufs), CHECK_RECV_FD))
            {
                ;
            }
        }
        cplus_socket_delete(sock);
    }
    return;
}

static void * event_server_initialize_object(
    struct cplus_event_server_config * config
    , CPLUS_EVENT_SERVER_CB_FUNCS cb_funcs)
{
    struct event_server * server = CPLUS_NULL;
    if ((server = (struct event_server *)cplus_malloc(sizeof(struct event_server))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(server);
        server->type = OBJ_TYPE_SERVER;
        server->enable_ipc = config->enable_ipc;
        server->skt_server = CPLUS_NULL;
        server->accept_proc = CPLUS_NULL;
        server->init_val = config->init_val;
        server->flag = config->flag;
        server->start = !!(config->start);
        server->cb_param = config->cb_param;
        if (cb_funcs)
        {
            if (cb_funcs->on_error)
            {
                server->on_error = cb_funcs->on_error;
            }
            if (cb_funcs->on_timeout)
            {
                server->on_timeout = cb_funcs->on_timeout;
            }
            if (cb_funcs->on_read)
            {
                server->on_read = cb_funcs->on_read;
            }
        }
        server->efd = eventfd(server->init_val, convert_efd_flag(server->flag));
        if (INVALID_FD == server->efd)
        {
            goto error;
        }
        server->poll_proc = cplus_task_new(
            event_poll_proc
            , server
            , cb_funcs
            , DURATION_FOR_POLL_PROC);
        if (CPLUS_NULL == server->poll_proc)
        {
            goto error;
        }
        if (server->enable_ipc)
        {
            server->skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL);
            if (CPLUS_NULL == server->skt_server)
            {
                goto error;
            }
            (void)cplus_socket_setopt_reuse_addr(server->skt_server, true);
            if (CPLUS_SUCCESS != cplus_socket_bind(server->skt_server, DEFAULT_EVENT_SERVER_NAME, 0))
            {
                goto error;
            }
            if (CPLUS_SUCCESS != cplus_socket_listen(server->skt_server, DEFAULT_LISTEN_COUNT))
            {
                goto error;
            }
            server->accept_proc = cplus_task_new(
                accept_proc
                , server
                , CPLUS_NULL
                , DURATION_FOR_ACCEPT_TASK);
            if (CPLUS_NULL == server->accept_proc)
            {
                goto error;
            }
        }
        if (server->start)
        {
            if (server->accept_proc)
            {
                cplus_task_start(server->accept_proc, 10U);
            }
            if (server->poll_proc)
            {
                cplus_task_start(server->poll_proc, 10U);
            }
        }
    }
    return server;
error:
    cplus_event_server_delete(server);
    return CPLUS_NULL;
}

cplus_event_server cplus_event_server_new(
    uint32_t init_val
    , CPLUS_EVENT_SERVER_FLAG flag
    , CPLUS_EVENT_SERVER_CB_FUNCS cb_funcs)
{
    struct cplus_event_server_config config = {0};
    config.init_val = init_val;
    config.flag = flag;
    config.start = false;
    config.cb_param = CPLUS_NULL;

    return event_server_initialize_object(&config, cb_funcs);
}

cplus_event_server cplus_event_server_new_config(
    CPLUS_EVENT_SERVER_CONFIG config
    , CPLUS_EVENT_SERVER_CB_FUNCS cb_funcs)
{
    return event_server_initialize_object(config, cb_funcs);
}

cplus_event_client cplus_event_client_attach(cplus_event_server obj)
{
    struct event_client * client = CPLUS_NULL;
    struct event_server * server = (struct event_server *)(obj);
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_SERVER);

    if ((client = (struct event_client *)cplus_malloc(sizeof(struct event_client))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(client);
        client->type = OBJ_TYPE_CLIENT;
        client->server_efd = server->efd;
        if (INVALID_FD == client->server_efd)
        {
            goto error;
        }
        client->skt_client = CPLUS_NULL;
    }
    return client;
error:
    cplus_event_client_delete(client);
    return CPLUS_NULL;
}

cplus_event_client cplus_event_client_connect(void)
{
    struct event_client * client = CPLUS_NULL;
    int32_t recv_fd = INVALID_FD;
    if ((client = (struct event_client *)cplus_malloc(sizeof(struct event_client))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(client);
        client->type = OBJ_TYPE_CLIENT;
        client->server_efd = INVALID_FD;
        client->skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL);
        if (CPLUS_NULL == client->skt_client)
        {
            goto error;
        }
        if(CPLUS_SUCCESS != cplus_socket_connect(client->skt_client, DEFAULT_EVENT_SERVER_NAME, 0))
        {
            goto error;
        }
        if (CPLUS_SUCCESS != cplus_socket_recv_fd(client->skt_client, &recv_fd, TIMEOUT_FOR_RECV_FD))
        {
            goto error;
        }
        if (strlen(CHECK_RECV_FD) != cplus_socket_send(client->skt_client, (void *)(CHECK_RECV_FD), strlen(CHECK_RECV_FD)))
        {
            goto error;
        }
        client->server_efd = recv_fd;
        if (INVALID_FD == client->server_efd)
        {
            goto error;
        }
        if (CPLUS_SUCCESS == cplus_socket_delete(client->skt_client))
        {
            client->skt_client = CPLUS_NULL;
        }
    }
    return client;
error:
    cplus_event_client_delete(client);
    return CPLUS_NULL;
}

int32_t cplus_event_server_start(cplus_event_server obj)
{
    struct event_server * server = (struct event_server *)(obj);
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_SERVER);
    if (!(server->start))
    {
        if (server->accept_proc)
        {
            cplus_task_start(server->accept_proc, 10U);
        }
        if (server->poll_proc)
        {
            cplus_task_start(server->poll_proc, 10U);
        }
        server->start = true;
        return CPLUS_SUCCESS;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_event_server_get_efd(cplus_event_server obj)
{
    struct event_server * server = (struct event_server *)(obj);
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_SERVER);
    return server->efd;
}

int32_t cplus_event_client_add_count(cplus_event_client obj, uint64_t count)
{
    struct event_client * client = (struct event_client *)(obj);
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_CLIENT);

    if (INVALID_FD == client->server_efd)
    {
        return CPLUS_FAIL;
    }
    if (sizeof(uint64_t) == write(client->server_efd, &count, sizeof(uint64_t)))
    {
        return CPLUS_SUCCESS;
    }
    return CPLUS_FAIL;
}

bool cplus_event_server_check(cplus_object obj)
{
    return (GET_OBJECT_TYPE(obj) == OBJ_TYPE_SERVER);
}

bool cplus_event_client_check(cplus_object obj)
{
    return (GET_OBJECT_TYPE(obj) == OBJ_TYPE_CLIENT);
}

#ifdef __CPLUS_UNITTEST__
#define WRITTEN_COUNT 5
#define RUN_COUNT 10

int32_t verification_count = 0, callback_param = 0;

int32_t on_read(int32_t fd, int32_t value, void * cb_param)
{
    verification_count += 5;
    return 0;
}

int32_t on_read_ex(int32_t fd, int32_t value, void * cb_param)
{
    int32_t * param = (int32_t *)(cb_param);
    verification_count += 5;
    (* param ) += 5;
    return 0;
}

CPLUS_UNIT_TEST(cplus_event_server_new, functionity)
{
    cplus_event_server server = CPLUS_NULL;
    cplus_event_client client = CPLUS_NULL;
    CPLUS_EVENT_SERVER_CB_FUNCS_T on_funcs = {0};
    verification_count = 0;

    on_funcs.on_read = on_read;
    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (server = cplus_event_server_new(
        0
        , CPLUS_EVENT_SERVER_FLAG_NONBLOCK
        , &on_funcs))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_start(server));
    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (client = cplus_event_client_attach(server))));
    for (int32_t i = 0; i < RUN_COUNT; i++)
    {
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_add_count(client, WRITTEN_COUNT));
        cplus_systime_sleep_msec(1000);
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_delete(client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_delete(server));
    UNITTEST_EXPECT_EQ(true, ((RUN_COUNT * WRITTEN_COUNT) == verification_count));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_event_server_new, CPLUS_SOCKET_TYPE_STREAM_LOCAL)
{
    cplus_event_server server = CPLUS_NULL;
    cplus_event_client client = CPLUS_NULL;
    struct cplus_event_server_config config = {0};
    CPLUS_EVENT_SERVER_CB_FUNCS_T on_funcs = {0};
    verification_count = 0;

    config.enable_ipc = true;
    config.init_val = 0;
    config.flag = CPLUS_EVENT_SERVER_FLAG_NONBLOCK;
    config.cb_param = &callback_param;

    on_funcs.on_read = on_read_ex;

    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (server = cplus_event_server_new_config(
        &config
        , &on_funcs))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_start(server));
    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (client = cplus_event_client_connect())));
    for (int32_t i = 0; i < RUN_COUNT; i++)
    {
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_add_count(client, WRITTEN_COUNT));
        cplus_systime_sleep_msec(1000);
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_delete(client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_delete(server));
    UNITTEST_EXPECT_EQ(true, ((RUN_COUNT * WRITTEN_COUNT) == verification_count));
    UNITTEST_EXPECT_EQ(true, ((RUN_COUNT * WRITTEN_COUNT) == callback_param));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_event_server(void)
{
    UNITTEST_ADD_TESTCASE(cplus_event_server_new, functionity);
    UNITTEST_ADD_TESTCASE(cplus_event_server_new, CPLUS_SOCKET_TYPE_STREAM_LOCAL);
}
#endif
