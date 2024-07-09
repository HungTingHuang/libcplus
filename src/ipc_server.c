/******************************************************************
* @file: ipc_server.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <arpa/inet.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_llist.h"
#include "cplus_mempool.h"
#include "cplus_mutex.h"
#include "cplus_socket.h"
#include "cplus_task.h"
#include "cplus_systime.h"
#include "cplus_pevent.h"
#include "cplus_ipc_server.h"

#define OBJ_TYPE_SERVER (OBJ_NONE + SYS + 7)
#define OBJ_TYPE_CLIENT (OBJ_NONE + SYS + 8)
#define TIMEOUT_FOR_STOP_IPC_SEVR_TASK (5 * 1000)
#define TIMEOUT_FOR_STOP_IPC_CONN_TASK 500
#define DURATION_FOR_IPC_SEVR_ACCEPT_TASK 10U
#define DURATION_FOR_IPC_CONN_TASK 10U
#define DEFAULT_RECEVICE_BUFFER_SIZE 512U
#define MAX_PACKET_DATA_SIZE (16 * DEFAULT_RECEVICE_BUFFER_SIZE)
#define DEFAULT_RESPONSE_BUFS_SIZE 8192U
#define DEFAULT_PACKET_DATA_SIZE 512U
#define TIMEOUT_FOR_HEARTBEAT_PACKET 3000
#define IPC_CONN_PACKET_TAG_SIZE 3U
#define TOLERANCE_TO_DIFF_SEQUENCE 3U
#define ACCUMULATE_SEQUENCE_NUMBER(SEQN) \
    ({ SEQN = ((255U <= SEQN)? 0: SEQN + 1U); SEQN; })

static uint8_t IPC_CONN_PACKET_BEGIN_TAG[] = "STX";
static uint8_t IPC_CONN_PACKET_END_TAG[] = "ETX";

typedef enum ipc_conn_statusus
{
    IPC_CONN_STATUS_NOT_CONNECTED = 0,
    IPC_CONN_STATUS_CONNECTED,
    IPC_CONN_STATUS_TIMEOUT,
    IPC_CONN_STATUS_FAULT,
} IPC_CONN_STATUS;

typedef enum ipc_conn_recv_stage
{
    IPC_CONN_STAGE_RECV_HEAD = 0,
    IPC_CONN_STAGE_RECV_SEQN,
    IPC_CONN_STAGE_RECV_CMD,
    IPC_CONN_STAGE_RECV_DATA_LEN,
    IPC_CONN_STAGE_RECV_DATA,
    IPC_CONN_STAGE_RECV_TAIL,
    IPC_CONN_STAGE_RECV_COMPLETED,
} IPC_CONN_RECV_STAGE;

typedef enum ipc_cmd
{
    IPC_CMD_HEARTBEAT = 0,
    IPC_CMD_ONEWAY,
    IPC_CMD_REQUEST,
    IPC_CMD_RESPONSE,
    IPC_CMD_ACK,
    IPC_CMD_MAX,
} IPC_CMD;

typedef struct ipc_conn_packet
{
    uint8_t seqn;
    uint8_t cmd;
    uint32_t data_len;
    void * data;
} *IPC_CONN_PACKET, IPC_CONN_PACKET_T;

typedef struct ipc_conn
{
    struct ipc_server * ipc_serv;
    bool is_async;
    uint32_t recv_timeout;
    volatile IPC_CONN_STATUS status;
    uint32_t sock_error;
    volatile IPC_CONN_RECV_STAGE recv_stage;
    cplus_socket sock;
    cplus_task conn_task;
    cplus_pevent evt_packet_received;
    IPC_CONN_PACKET_T packet;
    int32_t recv_count;
    uint8_t * recv_bufs;
    uint32_t recv_bufs_size;
    uint32_t recv_bufs_offset;
    uint32_t sub_offset;
    void * packet_databufs;
    uint32_t packet_databufs_size;
    uint32_t packet_databufs_offset;
    uint32_t response_data_size;
    void * response_data;
    CPLUS_IPC_CB_ON_RECEIVED on_received;
    CPLUS_IPC_CB_ON_DISCONNECTED on_disconnected;
    CPLUS_IPC_CB_ON_ERROR on_error;
} *IPC_CONN, IPC_CONN_T;

typedef struct ipc_server
{
    uint16_t type;
    uint32_t max_conn;
    cplus_socket accept_socket;
    cplus_mutex ipc_conn_sect;
    cplus_llist ipc_conn_list;
    cplus_mempool ipc_conn_pool;
    cplus_task accept_task;
    CPLUS_IPC_CB_ON_CONNECTED on_connected;
    CPLUS_IPC_CB_ON_ERROR on_error;
    CPLUS_IPC_CB_ON_DISCONNECTED on_disconnected;
    CPLUS_IPC_CB_ON_RECEIVED on_received;
} *IPC_SERVER, IPC_SERVER_T;

typedef struct ipc_client
{
    uint16_t type;
    bool is_async;
    cplus_socket server_socket;
    uint32_t seqn;
    IPC_CONN ipc_conn;
    CPLUS_IPC_CB_ON_CONNECTED on_connected;
} *IPC_CLIENT, IPC_CLIENT_T;

static int32_t ipc_conn_delete(IPC_CONN ipc_conn)
{
    if (ipc_conn)
    {
        ipc_conn->status = IPC_CONN_STATUS_NOT_CONNECTED;

        if (ipc_conn->conn_task)
        {
            cplus_task_stop(ipc_conn->conn_task, TIMEOUT_FOR_STOP_IPC_CONN_TASK);
        }

        if (ipc_conn->recv_bufs)
        {
            cplus_free(ipc_conn->recv_bufs);
        }

        if (ipc_conn->evt_packet_received)
        {
            cplus_pevent_delete(ipc_conn->evt_packet_received);
        }

        if (ipc_conn->response_data)
        {
            cplus_free(ipc_conn->response_data);
        }

        if (ipc_conn->packet_databufs)
        {
            cplus_free(ipc_conn->packet_databufs);
        }

        if (ipc_conn->sock)
        {
            cplus_socket_delete(ipc_conn->sock);
        }

        if (ipc_conn->ipc_serv)
        {
            cplus_mempool_free((ipc_conn->ipc_serv)->ipc_conn_pool, ipc_conn);
        }
        else
        {
            cplus_free(ipc_conn);
        }
    }

    return CPLUS_SUCCESS;
}

static int32_t ipc_client_delete(IPC_CLIENT ipc_clt)
{
    if (ipc_clt)
    {
        if (ipc_clt->ipc_conn)
        {
            ipc_conn_delete(ipc_clt->ipc_conn);
        }

        cplus_free(ipc_clt);
    }
    return CPLUS_SUCCESS;
}

static int32_t ipc_server_delete(IPC_SERVER ipc_serv)
{
    if (ipc_serv)
    {
        if (ipc_serv->accept_task)
        {
            cplus_task_stop(ipc_serv->accept_task, TIMEOUT_FOR_STOP_IPC_SEVR_TASK);
        }

        if (ipc_serv->ipc_conn_list)
        {
            IPC_CONN conn = NULL;
            while (NULL != (conn = cplus_llist_pop_back(ipc_serv->ipc_conn_list)))
            {
                ipc_conn_delete(conn);
                cplus_mempool_free(ipc_serv->ipc_conn_pool, conn);
            }
            cplus_llist_delete(ipc_serv->ipc_conn_list);
        }

        if (ipc_serv->ipc_conn_pool)
        {
            cplus_mempool_delete(ipc_serv->ipc_conn_pool);
        }

        if (ipc_serv->ipc_conn_sect)
        {
            cplus_mutex_delete(ipc_serv->ipc_conn_sect);
        }

        if (ipc_serv->accept_socket)
        {
            cplus_socket_delete(ipc_serv->accept_socket);
        }

        cplus_free(ipc_serv);
    }

    return CPLUS_SUCCESS;
}

int32_t cplus_ipc_server_delete(cplus_ipc_server obj)
{
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_SERVER);
    return ipc_server_delete(obj);
}

int32_t cplus_ipc_client_delete(cplus_ipc_client obj)
{
    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_CLIENT);
    return ipc_client_delete(obj);
}

int32_t ipc_recv_packet(
    cplus_socket skt
    , IPC_CONN_PACKET packet
    , uint32_t output_bufs_len
    , void * output_bufs
    , uint32_t timeout)
{
    int32_t res = CPLUS_FAIL, recv_count = 0;
    uint8_t begin_tag[3] = {0}, end_tag[3] = {0};
    uint32_t curr_tick = 0;
    void * recv_data = NULL;

    CHECK_NOT_NULL(skt, CPLUS_FAIL);
    CHECK_NOT_NULL(packet, CPLUS_FAIL);

    do
    {
        curr_tick = cplus_systime_get_tick();
        if (IPC_CONN_PACKET_TAG_SIZE != cplus_socket_recv(skt, begin_tag, IPC_CONN_PACKET_TAG_SIZE, timeout))
        {
            break;
        }
        timeout -= cplus_systime_elapsed_tick(curr_tick);

        if (0 != memcmp(IPC_CONN_PACKET_BEGIN_TAG, begin_tag, IPC_CONN_PACKET_TAG_SIZE))
        {
            break;
        }

        curr_tick = cplus_systime_get_tick();
        if (sizeof(packet->seqn) != cplus_socket_recv(skt, &(packet->seqn), sizeof(packet->seqn), timeout))
        {
            break;
        }
        timeout -= cplus_systime_elapsed_tick(curr_tick);

        curr_tick = cplus_systime_get_tick();
        if (sizeof(packet->cmd) != cplus_socket_recv(skt, &(packet->cmd), sizeof(packet->cmd), timeout))
        {
            break;
        }
        timeout -= cplus_systime_elapsed_tick(curr_tick);

        curr_tick = cplus_systime_get_tick();
        if (sizeof(packet->data_len) != cplus_socket_recv(skt, &(packet->data_len), sizeof(packet->data_len), timeout))
        {
            break;
        }
        timeout -= cplus_systime_elapsed_tick(curr_tick);
        packet->data_len = ntohl(packet->data_len);

        curr_tick = cplus_systime_get_tick();
        if (0 < packet->data_len and NULL != output_bufs and 0 < output_bufs_len)
        {
            recv_data = (void *)cplus_malloc(packet->data_len * sizeof(uint8_t));
            if (recv_data)
            {
                recv_count = cplus_socket_recv(skt, recv_data, packet->data_len, timeout);
                if (0 >= recv_count or packet->data_len != ((uint32_t)recv_count))
                {
                    break;
                }
                cplus_mem_cpy_ex(output_bufs, output_bufs_len, recv_data, packet->data_len);

                cplus_free(recv_data);
                timeout -= cplus_systime_elapsed_tick(curr_tick);
            }
        }

        curr_tick = cplus_systime_get_tick();
        if (IPC_CONN_PACKET_TAG_SIZE != cplus_socket_recv(skt, end_tag, sizeof(end_tag), timeout))
        {
            break;
        }
        timeout -= cplus_systime_elapsed_tick(curr_tick);

        if (0 != memcmp(IPC_CONN_PACKET_END_TAG, end_tag, IPC_CONN_PACKET_TAG_SIZE))
        {
            break;
        }

        res = recv_count;
    } while (0);

    if (recv_data)
    {
        cplus_free(recv_data);
    }
    return res;
}

int32_t ipc_send_packet(cplus_socket skt, IPC_CONN_PACKET packet)
{
    int32_t res = CPLUS_FAIL, send_count = 0;
    uint32_t network_order = 0;

    CHECK_NOT_NULL(skt, CPLUS_FAIL);
    CHECK_NOT_NULL(packet, CPLUS_FAIL);
    /* CHECK_GT_ZERO(packet->seqn, CPLUS_FAIL); */
    CHECK_IN_INTERVAL(((int32_t)packet->cmd), IPC_CMD_HEARTBEAT, (IPC_CMD_MAX - 1), CPLUS_FAIL);
    if (0 < packet->data_len)
    {
        CHECK_NOT_NULL(packet->data, CPLUS_FAIL);
    }

    do
    {
        if (IPC_CONN_PACKET_TAG_SIZE != cplus_socket_send(skt, IPC_CONN_PACKET_BEGIN_TAG, IPC_CONN_PACKET_TAG_SIZE))
        {
            break;
        }

        if (sizeof(packet->seqn) != cplus_socket_send(skt, &(packet->seqn), sizeof(packet->seqn)))
        {
            break;
        }

        if (sizeof(packet->cmd) != cplus_socket_send(skt, &(packet->cmd), sizeof(packet->cmd)))
        {
            break;
        }

        /* Convert network byte order. */
        network_order = htonl(packet->data_len);
        if (sizeof(packet->data_len) != cplus_socket_send(skt, &network_order, sizeof(packet->data_len)))
        {
            break;
        }

        if (0 < packet->data_len and NULL != packet->data)
        {
            send_count = cplus_socket_send(skt, packet->data, packet->data_len);
            if (0 > send_count or packet->data_len != ((uint32_t)send_count))
            {
                break;
            }
        }

        if (IPC_CONN_PACKET_TAG_SIZE != cplus_socket_send(skt, IPC_CONN_PACKET_END_TAG, IPC_CONN_PACKET_TAG_SIZE))
        {
            break;
        }

        res = send_count;
    } while (0);

    return res;
}

static int32_t packet_analyze_completed(IPC_CONN ipc_conn)
{
    int32_t res = CPLUS_SUCCESS;
    uint32_t dataout_size = ipc_conn->response_data_size;
    IPC_CONN_PACKET completed_packet = &(ipc_conn->packet);
    IPC_CONN_PACKET_T response_packet = {0};

    switch(completed_packet->cmd)
    {
    default:
    case IPC_CMD_ACK:
        break;
    case IPC_CMD_HEARTBEAT:
        response_packet.seqn = completed_packet->seqn;
        response_packet.cmd = IPC_CMD_ACK;
        response_packet.data_len = 0;
        response_packet.data = NULL;
        res = ipc_send_packet(ipc_conn->sock, &response_packet);
        break;
    case IPC_CMD_ONEWAY:
    case IPC_CMD_REQUEST:
    case IPC_CMD_RESPONSE:
        if (ipc_conn->on_received)
        {
            if (NULL == ipc_conn->response_data)
            {
                ipc_conn->response_data = (void *)cplus_malloc(ipc_conn->response_data_size);
                if (NULL == ipc_conn->response_data)
                {
                    return CPLUS_FAIL;
                }
                cplus_mem_set(ipc_conn->response_data, 0x00, ipc_conn->response_data_size);
            }

            res = ipc_conn->on_received(
                ipc_conn->sock
                , completed_packet->data_len
                , completed_packet->data
                , &dataout_size /* Pass current size of 'response_data'. */
                , ipc_conn->response_data);

            if (CPLUS_FAIL == res)
            {
                return CPLUS_FAIL;
            }

            /* The needed buffer size is larger than the size of existing buffer. */
            if (dataout_size > ipc_conn->response_data_size)
            {
                /* Re-allocate new buffer size based on needed size. */
                ipc_conn->response_data = (void *)cplus_realloc(
                    ipc_conn->response_data
                    , dataout_size);
                if (NULL == ipc_conn->response_data)
                {
                    return CPLUS_FAIL;
                }
                cplus_mem_set(ipc_conn->response_data, 0x00, dataout_size);

                /* Update and store existing size. */
                ipc_conn->response_data_size = dataout_size;

                /* Invoke on_received() callback function to get compleled data. */
                res = ipc_conn->on_received(
                    ipc_conn->sock
                    , completed_packet->data_len
                    , completed_packet->data
                    , &(ipc_conn->response_data_size)
                    , ipc_conn->response_data);

                if (CPLUS_FAIL == res)
                {
                    return CPLUS_FAIL;
                }
            }

            if (IPC_CMD_REQUEST == completed_packet->cmd)
            {
                if (0 != ipc_conn->response_data_size)
                {
                    response_packet.seqn = completed_packet->seqn;
                    response_packet.cmd = IPC_CMD_RESPONSE;
                    response_packet.data_len = dataout_size;
                    response_packet.data = ipc_conn->response_data;
                    res = ipc_send_packet(ipc_conn->sock, &response_packet);
                }
            }
            else if (IPC_CMD_RESPONSE == completed_packet->cmd)
            {
                response_packet.seqn = completed_packet->seqn;
                response_packet.cmd = IPC_CMD_ACK;
                response_packet.data_len = 0;
                response_packet.data = NULL;
                res = ipc_send_packet(ipc_conn->sock, &response_packet);
            }
        }
        break;
    }

    return res;
}

static void ipc_packet_analyze(
    IPC_CONN ipc_conn
    , int32_t (* on_completed)(IPC_CONN))
{
    uint32_t diff = 0, walker_count = (((uint32_t)ipc_conn->recv_count) + ipc_conn->recv_bufs_offset);
    IPC_CONN_PACKET packet = &(ipc_conn->packet);

    while (ipc_conn->recv_bufs_offset < walker_count)
    {
        switch (ipc_conn->recv_stage)
        {
        default:
        case IPC_CONN_STAGE_RECV_HEAD:
            if (IPC_CONN_PACKET_BEGIN_TAG[ipc_conn->sub_offset] == ((uint8_t *)ipc_conn->recv_bufs)[ipc_conn->recv_bufs_offset])
            {
                ipc_conn->sub_offset += sizeof(uint8_t);
                if (IPC_CONN_PACKET_TAG_SIZE <= ipc_conn->sub_offset)
                {
                    /* Recvice a head tag, hence reset event fo receive a packet. */
                    if (ipc_conn->evt_packet_received)
                    {
                        cplus_pevent_reset(ipc_conn->evt_packet_received);
                    }

                    ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_SEQN;
                    ipc_conn->sub_offset = 0;
                }
            }
            else
            {
                /* Drop this packet, re-pick a new one. */
                ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_HEAD;
                ipc_conn->sub_offset = 0;
            }
            ipc_conn->recv_bufs_offset += sizeof(uint8_t);
            break;
        case IPC_CONN_STAGE_RECV_SEQN:
            packet->seqn = ((uint8_t *)ipc_conn->recv_bufs)[ipc_conn->recv_bufs_offset];
            ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_CMD;
            ipc_conn->recv_bufs_offset += sizeof(uint8_t);
            break;
        case IPC_CONN_STAGE_RECV_CMD:
            packet->cmd = ((uint8_t *)ipc_conn->recv_bufs)[ipc_conn->recv_bufs_offset];
            ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_DATA_LEN;
            ipc_conn->recv_bufs_offset += sizeof(uint8_t);
            break;
        case IPC_CONN_STAGE_RECV_DATA_LEN:
            if (0 == ipc_conn->sub_offset)
            {
                packet->data_len = 0;
            }

            if (sizeof(packet->data_len) > ipc_conn->sub_offset)
            {
                packet->data_len <<= (ipc_conn->sub_offset? 8: 0);
                packet->data_len += ((uint8_t *)ipc_conn->recv_bufs)[ipc_conn->recv_bufs_offset];
                ipc_conn->sub_offset += sizeof(uint8_t);
                ipc_conn->recv_bufs_offset += sizeof(uint8_t);
            }
            else
            {
                ipc_conn->sub_offset = 0;

                if (MAX_PACKET_DATA_SIZE < packet->data_len)
                {
                    /* Drop this packet, re-pick a new one. */
                    ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_HEAD;
                    ipc_conn->sub_offset = 0;
                }

                if (0 < packet->data_len)
                {
                    ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_DATA;
                }
                else
                {
                    ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_TAIL;
                }
            }
            break;
        case IPC_CONN_STAGE_RECV_DATA:
            if (packet->data_len > ipc_conn->packet_databufs_size)
            {
                if (NULL == ipc_conn->packet_databufs)
                {
                    ipc_conn->packet_databufs = (void *)cplus_malloc(packet->data_len);
                }
                else
                {
                    ipc_conn->packet_databufs = (void *)cplus_realloc(
                        ipc_conn->packet_databufs
                        , packet->data_len);
                }

                if (ipc_conn->packet_databufs)
                {
                    ipc_conn->packet_databufs_size = packet->data_len;
                    cplus_mem_set(ipc_conn->packet_databufs, 0x00, ipc_conn->packet_databufs_size);
                }
            }
            else
            {
                if (NULL == ipc_conn->packet_databufs)
                {
                    ipc_conn->packet_databufs = (void *)cplus_malloc(ipc_conn->packet_databufs_size);
                    cplus_mem_set(ipc_conn->packet_databufs, 0x00, ipc_conn->packet_databufs_size);
                }
            }

            if (NULL == ipc_conn->packet_databufs)
            {
                /* Cannot allocate memory
                Drop this packet, re-pick a new one. */
                ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_HEAD;
                ipc_conn->sub_offset = 0;
            }
            else
            {
                packet->data = ipc_conn->packet_databufs;

                diff = CPLUS_MIN(
                    (packet->data_len - ipc_conn->packet_databufs_offset)
                    , (ipc_conn->recv_bufs_size - ipc_conn->recv_bufs_offset));

                cplus_mem_cpy_ex(
                    &(((uint8_t *)packet->data)[ipc_conn->packet_databufs_offset])
                    , diff
                    , &(ipc_conn->recv_bufs[ipc_conn->recv_bufs_offset])
                    , diff);

                ipc_conn->packet_databufs_offset += diff;
                ipc_conn->recv_bufs_offset += diff;

                if (0 >= (packet->data_len - ipc_conn->packet_databufs_offset))
                {
                    ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_TAIL;
                    ipc_conn->packet_databufs_offset = 0;
                }
            }
            break;
        case IPC_CONN_STAGE_RECV_TAIL:
            if (IPC_CONN_PACKET_END_TAG[ipc_conn->sub_offset] == ((uint8_t *)ipc_conn->recv_bufs)[ipc_conn->recv_bufs_offset])
            {
                ipc_conn->sub_offset += sizeof(uint8_t);
                if (IPC_CONN_PACKET_TAG_SIZE <= ipc_conn->sub_offset)
                {
                    ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_HEAD;
                    ipc_conn->sub_offset = 0;

                    if (on_completed)
                    {
                        on_completed(ipc_conn);
                    }

                    if (ipc_conn->evt_packet_received)
                    {
                        cplus_pevent_set(ipc_conn->evt_packet_received);
                        if (true == ipc_conn->is_async)
                        {
                            /* Re-pick another new packet. */
                            break;
                        }
                    }
                }
            }
            else
            {
                /* Drop this packet, re-pick a new one. */
                ipc_conn->recv_stage = IPC_CONN_STAGE_RECV_HEAD;
                ipc_conn->sub_offset = 0;
            }
            ipc_conn->recv_bufs_offset += sizeof(uint8_t);
            break;
        }
    }
}

static void ipc_conn_proc(void * param1, void * param2)
{
    IPC_CONN ipc_conn = (IPC_CONN)param1;
    UNUSED_PARAM(param2);

    if (IPC_CONN_STATUS_NOT_CONNECTED == ipc_conn->status)
    {
        return;
    }

    if (ipc_conn->recv_bufs_offset >= ipc_conn->recv_bufs_size)
    {
        ipc_conn->recv_bufs_offset = 0;
    }

    ipc_conn->recv_count = cplus_socket_recv(
        ipc_conn->sock
        , &(((uint8_t *)ipc_conn->recv_bufs)[ipc_conn->recv_bufs_offset])
        , (ipc_conn->recv_bufs_size - ipc_conn->recv_bufs_offset)
        , ipc_conn->recv_timeout);

    if (0 == ipc_conn->recv_count)
    {
        if (ipc_conn->ipc_serv)
        {
            cplus_crit_sect_enter((ipc_conn->ipc_serv)->ipc_conn_sect);
        }

        if (ipc_conn->on_disconnected)
        {
            ipc_conn->on_disconnected(ipc_conn->sock);
        }

        cplus_socket_delete(ipc_conn->sock);
        ipc_conn->sock = NULL;
        ipc_conn->status = IPC_CONN_STATUS_NOT_CONNECTED;

        if (ipc_conn->ipc_serv)
        {
            cplus_crit_sect_exit((ipc_conn->ipc_serv)->ipc_conn_sect);
        }
    }
    else if (0 > ipc_conn->recv_count)
    {
        ipc_conn->sock_error = errno;

        if (ipc_conn->ipc_serv)
        {
            cplus_crit_sect_enter((ipc_conn->ipc_serv)->ipc_conn_sect);
        }

        switch (ipc_conn->sock_error)
        {
        default:
        case EBADF:
            /* The argument sockfd is an invalid file descriptor. */
        case EFAULT:
            /* The receive buffer pointer(s) point outside the process's
            address space. */
        case EINVAL:
            /* Invalid argument passed. */
        case ENOMEM:
            /* Could not allocate memory for recv(). */
            ipc_conn->status = IPC_CONN_STATUS_FAULT;
            break;
        case ETIMEDOUT:
            /* Timeout */
            ipc_conn->status = IPC_CONN_STATUS_TIMEOUT;
            break;
        case ENOTCONN:
            /* The socket is associated with a connection-oriented
            protocol and has not been connected */
            ipc_conn->status = IPC_CONN_STATUS_NOT_CONNECTED;
            break;
        case EINTR:
            /* The receive was interrupted by delivery of a signal before
            any data was available. */
            break;
        }

        if (ipc_conn->ipc_serv)
        {
            cplus_crit_sect_exit((ipc_conn->ipc_serv)->ipc_conn_sect);
        }

        if (ipc_conn->on_error)
        {
            ipc_conn->on_error(ipc_conn->sock, ipc_conn->sock_error);
        }
    }
    else
    {
        /* Received the packet successfully, so refresh the status. */
        ipc_conn->sock_error = 0;
        ipc_conn->status = IPC_CONN_STATUS_CONNECTED;

        ipc_packet_analyze(
            ipc_conn
            , packet_analyze_completed);
    }
    return;
}

static IPC_CONN ipc_conn_create(
    IPC_SERVER ipc_serv
    , cplus_socket * conn_sock
    , bool is_async
    , CPLUS_IPC_CB_ON_RECEIVED on_received
    , CPLUS_IPC_CB_ON_DISCONNECTED on_disconnected
    , CPLUS_IPC_CB_ON_ERROR on_error)
{
    IPC_CONN conn = NULL;

    conn = (IPC_CONN)((ipc_serv)? cplus_mempool_alloc(ipc_serv->ipc_conn_pool): cplus_malloc(sizeof(struct ipc_conn)));
    if (conn)
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(conn);

        conn->ipc_serv = ipc_serv;
        conn->is_async = is_async;
        conn->recv_timeout = CPLUS_INFINITE_TIMEOUT;
        conn->sock = conn_sock;
        conn->evt_packet_received = NULL;
        conn->status = IPC_CONN_STATUS_NOT_CONNECTED;
        conn->sock_error = 0;
        conn->recv_stage = IPC_CONN_STAGE_RECV_HEAD;
        conn->recv_count = 0;
        conn->recv_bufs = NULL;
        conn->recv_bufs_size = DEFAULT_RECEVICE_BUFFER_SIZE;
        conn->recv_bufs_offset = 0;
        conn->sub_offset = 0;
        conn->packet_databufs = NULL;
        conn->packet_databufs_size = DEFAULT_PACKET_DATA_SIZE;
        conn->packet_databufs_offset = 0;
        conn->response_data_size = DEFAULT_RESPONSE_BUFS_SIZE;
        conn->response_data = NULL;
        conn->conn_task = NULL;

        if (on_received)
        {
            conn->on_received = on_received;
        }
        if (on_disconnected)
        {
            conn->on_disconnected = on_disconnected;
        }
        if (on_error)
        {
            conn->on_error = on_error;
        }

        if (NULL == conn->recv_bufs)
        {
            conn->recv_bufs = (uint8_t *)cplus_malloc(conn->recv_bufs_size);
            if (NULL == conn->recv_bufs)
            {
                goto error;
            }
            cplus_mem_set(conn->recv_bufs, 0x00, conn->recv_bufs_size);
        }

        if (true == conn->is_async)
        {
            conn->conn_task = cplus_task_new(
                ipc_conn_proc
                , conn
                , NULL
                , DURATION_FOR_IPC_CONN_TASK);
            if (NULL == conn->conn_task)
            {
                goto error;
            }
            cplus_task_start(conn->conn_task, 500);
        }
        else
        {
            if (NULL == conn->evt_packet_received)
            {
                conn->evt_packet_received = cplus_pevent_new(true, false);
                if (NULL == conn->evt_packet_received)
                {
                    goto error;
                }
            }
        }
        return conn;
    }
error:
    ipc_conn_delete(conn);
    return NULL;
}

static inline int32_t find_disconnect_conn(void * data, void * arg)
{
    UNUSED_PARAM(arg);
    return !(IPC_CONN_STATUS_NOT_CONNECTED == ((IPC_CONN)data)->status);
}

void ipc_server_proc(void * param1, void * param2)
{
    IPC_SERVER ipc_serv = (IPC_SERVER)param1;
    bool can_accept = false;
    IPC_CONN conn = NULL;
    cplus_socket sock = NULL;
    UNUSED_PARAM(param2);

    cplus_crit_sect_enter(ipc_serv->ipc_conn_sect);
    while ((conn = (IPC_CONN)cplus_llist_pop_if(
        ipc_serv->ipc_conn_list
        , find_disconnect_conn
        , NULL)))
    {
        ipc_conn_delete(conn);
    }

    if (cplus_llist_get_size(ipc_serv->ipc_conn_list) < ipc_serv->max_conn)
    {
        can_accept = true;
    }
    cplus_crit_sect_exit(ipc_serv->ipc_conn_sect);

    if (true == can_accept)
    {
        if ((sock = cplus_socket_accept(ipc_serv->accept_socket, CPLUS_INFINITE_TIMEOUT)))
        {
            conn = ipc_conn_create(
                ipc_serv
                , sock
                , true
                , ipc_serv->on_received
                , ipc_serv->on_disconnected
                , ipc_serv->on_error);
            if (conn)
            {
                conn->status = IPC_CONN_STATUS_CONNECTED;

                cplus_crit_sect_enter(ipc_serv->ipc_conn_sect);
                cplus_llist_push_front(ipc_serv->ipc_conn_list, conn);
                cplus_crit_sect_exit(ipc_serv->ipc_conn_sect);

                /* Invoke on_connect() callback function. */
                if (ipc_serv->on_connected)
                {
                    ipc_serv->on_connected(sock);
                }
            }
        }
    }
    return;
}

int32_t cplus_ipc_client_send_heartbeat(cplus_ipc_client obj, uint32_t timeout)
{
    int32_t res = CPLUS_FAIL, count = 0;
    IPC_CLIENT clt = (IPC_CLIENT)obj;
    IPC_CONN_PACKET_T request_packet = {0}, response_packet = {0};

    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_CLIENT);

    request_packet.seqn = ACCUMULATE_SEQUENCE_NUMBER(clt->seqn);
    request_packet.cmd = IPC_CMD_HEARTBEAT;
    request_packet.data_len = 0;
    request_packet.data = NULL;

    count = ipc_send_packet(clt->server_socket, &request_packet);
    if (0 <= count)
    {
        count = ipc_recv_packet(clt->server_socket, &response_packet, 0, NULL, timeout);
        if (0 <= count)
        {
            if (request_packet.seqn == response_packet.seqn
                and IPC_CMD_ACK == response_packet.cmd)
            {
                res = CPLUS_SUCCESS;
            }
        }
    }
    return res;
}

int32_t cplus_ipc_client_send_oneway(
    cplus_ipc_client obj
    , uint32_t input_bufs_len
    , void * input_bufs)
{
    IPC_CLIENT clt = (IPC_CLIENT)obj;
    IPC_CONN_PACKET_T resquest_packet = {0};

    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_CLIENT);

    resquest_packet.seqn = ACCUMULATE_SEQUENCE_NUMBER(clt->seqn);
    resquest_packet.cmd = IPC_CMD_ONEWAY;
    resquest_packet.data_len = input_bufs_len;
    resquest_packet.data = input_bufs;

    return ipc_send_packet(clt->server_socket, &resquest_packet);
}

int32_t cplus_ipc_client_send_request(
    cplus_ipc_client obj
    , uint32_t input_bufs_len
    , void * input_bufs
    , uint32_t output_bufs_len
    , void * output_bufs
    , uint32_t timeout)
{
    int32_t res = CPLUS_SUCCESS, count = 0;
    IPC_CLIENT clt = (IPC_CLIENT)obj;
    IPC_CONN_PACKET_T resquest_packet = {0};

    CHECK_OBJECT_TYPE_EX(obj, OBJ_TYPE_CLIENT);

    resquest_packet.seqn = ACCUMULATE_SEQUENCE_NUMBER(clt->seqn);
    resquest_packet.cmd = IPC_CMD_REQUEST;
    resquest_packet.data_len = input_bufs_len;
    resquest_packet.data = input_bufs;

    count = ipc_send_packet(clt->server_socket, &resquest_packet);
    if (0 < count)
    {
        if (true == clt->is_async)
        {
            res = CPLUS_SUCCESS;
        }
        else
        {
            (clt->ipc_conn)->recv_timeout = timeout;

            while (true)
            {
                timeout = cplus_systime_get_tick();
                ipc_conn_proc(clt->ipc_conn, NULL);
                count = (clt->ipc_conn)->recv_count;
                if (0 >= count)
                {
                    res = CPLUS_FAIL;
                    break;
                }
                else
                {
                    uint32_t elapsed_period = cplus_systime_elapsed_tick(timeout);
                    if (elapsed_period >= (clt->ipc_conn)->recv_timeout)
                    {
                        errno = ETIMEDOUT;
                        res = CPLUS_FAIL;
                        break;
                    }
                    else
                    {
                        /* Refreah timeout. */
                        (clt->ipc_conn)->recv_timeout -= elapsed_period;
                    }
                }

                /* Reason for False Positive : The mutex will be locked or always blocked
                    until call calling pthread_cond_broadcast() or pthread_cond_signal(). */
                /* coverity[double_unlock: FALSE] */
                if (CPLUS_FAIL == cplus_pevent_wait((clt->ipc_conn)->evt_packet_received, 0))
                {
                    if (EAGAIN != errno)
                    {
                        res = CPLUS_FAIL;
                        break;
                    }
                }
                else
                {
                    /* Completely received a packet */
                    if (resquest_packet.seqn == clt->ipc_conn->packet.seqn)
                    {
                        /* Sequence number correct, then exit loop */
                        res = CPLUS_SUCCESS;
                        break;
                    }
                    else
                    {
                        uint32_t diff = 0;
                        if (resquest_packet.seqn > (clt->ipc_conn)->packet.seqn)
                        {
                            diff = resquest_packet.seqn - (clt->ipc_conn)->packet.seqn;
                        }
                        else
                        {
                            diff = (255U - resquest_packet.seqn) + (clt->ipc_conn)->packet.seqn;
                        }

                        if (TOLERANCE_TO_DIFF_SEQUENCE > diff)
                        {
                            /* Drop recviced packet */
                            cplus_pevent_reset((clt->ipc_conn)->evt_packet_received);
                        }
                        else
                        {
                            errno = EILSEQ;
                            res = CPLUS_FAIL;
                            break;
                        }
                    }
                }
            }

            cplus_pevent_reset((clt->ipc_conn)->evt_packet_received);

            if (CPLUS_FAIL == res
                or 0 != (clt->ipc_conn)->sock_error)
            {
                errno = (0 != (clt->ipc_conn)->sock_error)? (clt->ipc_conn)->sock_error: errno;
                res = CPLUS_FAIL;
            }
            else
            {
                CHECK_NOT_NULL(output_bufs, CPLUS_FAIL);
                CHECK_GT_ZERO(output_bufs_len, CPLUS_FAIL);

                cplus_mem_cpy_ex(
                    output_bufs
                    , output_bufs_len
                    , (clt->ipc_conn)->packet.data
                    , (clt->ipc_conn)->packet.data_len);

                res = (int32_t)((clt->ipc_conn)->packet.data_len);
            }
        }
    }
    return res;
}

static void * ipc_server_new(
    const char * name
    , uint32_t max_connection
    , CPLUS_IPC_CB_FUNCS cb_funcs)
{
    IPC_SERVER ipc_serv = (IPC_SERVER)cplus_malloc(sizeof(IPC_SERVER_T));
    if (ipc_serv)
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(ipc_serv);

        ipc_serv->type = OBJ_TYPE_SERVER;
        ipc_serv->max_conn = max_connection;

        ipc_serv->accept_socket = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL);
        if (NULL == ipc_serv->accept_socket)
        {
            goto exit;
        }

        (void)cplus_socket_setopt_reuse_addr(ipc_serv->accept_socket, true);

        if (CPLUS_SUCCESS != cplus_socket_bind(ipc_serv->accept_socket, name, 0))
        {
            goto exit;
        }

        ipc_serv->ipc_conn_list = cplus_llist_prev_new(ipc_serv->max_conn);
        if (NULL == ipc_serv->ipc_conn_list)
        {
            goto exit;
        }

        ipc_serv->ipc_conn_pool = cplus_mempool_new(ipc_serv->max_conn, sizeof(struct ipc_conn));
        if (NULL == ipc_serv->ipc_conn_pool)
        {
            goto exit;
        }

        ipc_serv->ipc_conn_sect = cplus_mutex_new();
        if (NULL == ipc_serv->ipc_conn_sect)
        {
            goto exit;
        }

        if (cb_funcs)
        {
            if (cb_funcs->on_connected)
            {
                ipc_serv->on_connected = cb_funcs->on_connected;
            }
            if (cb_funcs->on_error)
            {
                ipc_serv->on_error = cb_funcs->on_error;
            }
            if (cb_funcs->on_disconnected)
            {
                ipc_serv->on_disconnected = cb_funcs->on_disconnected;
            }
            if (cb_funcs->on_received)
            {
                ipc_serv->on_received = cb_funcs->on_received;
            }
        }

        if (CPLUS_SUCCESS != cplus_socket_listen(ipc_serv->accept_socket, ipc_serv->max_conn))
        {
            goto exit;
        }

        ipc_serv->accept_task = cplus_task_new(
            ipc_server_proc
            , ipc_serv
            , NULL
            , DURATION_FOR_IPC_SEVR_ACCEPT_TASK);
        if (NULL == ipc_serv->accept_task)
        {
            goto exit;
        }

        cplus_task_start(ipc_serv->accept_task, 500);
        cplus_task_wait_start(ipc_serv->accept_task, CPLUS_INFINITE_TIMEOUT);

        return ipc_serv;
    }
exit:
    ipc_server_delete(ipc_serv);
    return NULL;
}

static void * ipc_client_new(
    const char * name
    , CPLUS_IPC_CB_FUNCS cb_funcs)
{
    IPC_CLIENT ipc_clt = NULL;

    ipc_clt = (IPC_CLIENT)cplus_malloc(sizeof(IPC_CLIENT_T));
    if (ipc_clt)
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(ipc_clt);

        ipc_clt->type = OBJ_TYPE_CLIENT;
        ipc_clt->is_async = false;
        ipc_clt->ipc_conn = NULL;
        ipc_clt->seqn = 0;

        ipc_clt->server_socket = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL);
        if (NULL == ipc_clt->server_socket)
        {
            goto error;
        }

        if (cb_funcs)
        {
            if (cb_funcs->on_connected)
            {
                ipc_clt->on_connected = cb_funcs->on_connected;
            }
            if (cb_funcs->on_received)
            {
                ipc_clt->is_async = true;
            }
        }

        if (CPLUS_SUCCESS != cplus_socket_connect(ipc_clt->server_socket, name, 0))
        {
            goto error;
        }
        else
        {
            if (CPLUS_FAIL == cplus_ipc_client_send_heartbeat(ipc_clt, TIMEOUT_FOR_HEARTBEAT_PACKET))
            {
                cplus_socket_delete(ipc_clt->server_socket);
                goto error;
            }
            else
            {
                ipc_clt->ipc_conn = ipc_conn_create(
                    NULL
                    , ipc_clt->server_socket
                    , ipc_clt->is_async
                    , NULL
                    , NULL
                    , NULL);
                if (NULL == ipc_clt->ipc_conn)
                {
                    goto error;
                }

                ipc_clt->ipc_conn->status = IPC_CONN_STATUS_CONNECTED;
                if (ipc_clt->on_connected)
                {
                    ipc_clt->on_connected(ipc_clt->server_socket);
                }
            }
        }

        return ipc_clt;
    }
error:
    ipc_client_delete(ipc_clt);
    return NULL;
}

cplus_ipc_server cplus_ipc_server_new(
    const char * name
    , uint32_t max_connection
    , CPLUS_IPC_CB_FUNCS cb_funcs)
{
    CHECK_NOT_NULL(name, NULL);
    CHECK_GT_ZERO(max_connection, NULL);
    CHECK_NOT_NULL(cb_funcs, NULL);

    return ipc_server_new(name, max_connection, cb_funcs);
}

cplus_ipc_client cplus_ipc_client_new(
    const char * name
    , CPLUS_IPC_CB_FUNCS cb_funcs)
{
    CHECK_NOT_NULL(name, NULL);

    return ipc_client_new(name, cb_funcs);
}

bool cplus_ipc_server_check(cplus_object obj)
{
    return (GET_OBJECT_TYPE(obj) == OBJ_TYPE_SERVER);
}

bool cplus_ipc_client_check(cplus_object obj)
{
    return (GET_OBJECT_TYPE(obj) == OBJ_TYPE_CLIENT);
}

#ifdef __CPLUS_UNITTEST__
#include "cplus_systime.h"

#define SERVER_NAME "unittest_serv"
#define MAX_CLIENT_COUNT 5
static uint32_t client_count = 0;
static uint32_t verification_count[5] = {0};

static char test_string0[] = "Hello Word";
static char test_string1[] = "AaBbCcDd";
static char test_string2[] = "";
static char test_string3[] = "1";
static char test_string4[] = \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789";

static char response_string0[] = "Dlrow olleh";
static char response_string1[] = "dDcCbCaA";
static char response_string2[] = "";
static char response_string3[] = "2";
static char response_string4[] = \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789" \
"012345678901234567890123456789012345678901234567890123456789";

int32_t serv_on_disconnected(cplus_socket conn_sock)
{
    client_count -= 1;
    return CPLUS_SUCCESS;
}

int32_t serv_on_connected(cplus_socket conn_sock)
{
    client_count += 1;
    return CPLUS_SUCCESS;
}

int32_t serv_request_on_received(
    cplus_socket conn_sock
    , uint32_t input_bufs_len
    , void * input_bufs
    , uint32_t * output_bufs_len
    , void * output_bufs)
{
    if (((strlen(test_string0) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string0)))
    {
        cplus_mem_cpy_ex(
            output_bufs
            , (*output_bufs_len)
            , response_string0
            , strlen(response_string0) + 1);
        if ((* output_bufs_len) >= (strlen(response_string0) + 1))
        {
            verification_count[0] += 1;
        }
        (*output_bufs_len) = strlen(response_string0) + 1;
    }
    else if (((strlen(test_string1) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string1)))
    {
        cplus_mem_cpy_ex(
            output_bufs
            , (*output_bufs_len)
            , response_string1
            , strlen(response_string1) + 1);
        if ((* output_bufs_len) >= (strlen(response_string1) + 1))
        {
            verification_count[1] += 1;
        }
        (*output_bufs_len) = strlen(response_string1) + 1;
    }
    else if (((strlen(test_string2) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string2)))
    {
        cplus_mem_cpy_ex(
            output_bufs
            , (*output_bufs_len)
            , response_string2
            , strlen(response_string2) + 1);
        if ((* output_bufs_len) >= (strlen(response_string2) + 1))
        {
            verification_count[2] += 1;
        }
        (*output_bufs_len) = strlen(response_string2) + 1;
    }
    else if (((strlen(test_string3) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string3)))
    {
        cplus_mem_cpy_ex(
            output_bufs
            , (*output_bufs_len)
            , response_string3
            , strlen(response_string3) + 1);
        if ((* output_bufs_len) >= (strlen(response_string3) + 1))
        {
            verification_count[3] += 1;
        }
        (*output_bufs_len) = strlen(response_string3) + 1;
    }
    else if (((strlen(test_string4) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string4)))
    {
        cplus_mem_cpy_ex(
            output_bufs
            , (*output_bufs_len)
            , response_string4
            , strlen(response_string4) + 1);
        if ((* output_bufs_len) >= (strlen(response_string4) + 1))
        {
            verification_count[4] += 1;
        }
        (* output_bufs_len) = strlen(response_string4) + 1;
    }
    return CPLUS_SUCCESS;
}

CPLUS_UNIT_TEST(CPLUS_IPC_CLIENT_SEND_REQUEST, functionity)
{
    client_count = 0;
    for (int i = 0; i < sizeof(verification_count)/sizeof(uint32_t); i++)
    {
        verification_count[i] = 0;
    }

    cplus_ipc_server ipc_server = NULL;
    cplus_ipc_client client[MAX_CLIENT_COUNT] = {0};

    char recv_bufs[65536] = {0};
    uint32_t recv_bufs_size = sizeof(recv_bufs);
    int32_t recv_count = 0;

    CPLUS_IPC_CB_FUNCS_T ipc_server_cb_funcs = {0};
    ipc_server_cb_funcs.on_connected = serv_on_connected;
    ipc_server_cb_funcs.on_received = serv_request_on_received;
    ipc_server_cb_funcs.on_disconnected = serv_on_disconnected;

    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_server = cplus_ipc_server_new(SERVER_NAME, MAX_CLIENT_COUNT, &ipc_server_cb_funcs))));
    for (int32_t idx = 0; idx < MAX_CLIENT_COUNT; idx++)
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (client[idx] = cplus_ipc_client_new(SERVER_NAME, NULL))));
        UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                client[idx]
                , strlen(test_string0) + 1
                , test_string0
                , recv_bufs_size
                , (void *)recv_bufs
                , 10000))));
        UNITTEST_EXPECT_EQ(strlen(response_string0) + 1, recv_count);
        UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string0));
        UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                client[idx]
                , strlen(test_string1) + 1
                , test_string1
                , recv_bufs_size
                , (void *)recv_bufs
                , 10000))));
        UNITTEST_EXPECT_EQ(strlen(response_string1) + 1, recv_count);
        UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string1));
        UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                client[idx]
                , strlen(test_string2) + 1
                , test_string2
                , recv_bufs_size
                , (void *)recv_bufs
                , 10000))));
        UNITTEST_EXPECT_EQ(strlen(response_string2) + 1, recv_count);
        UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string2));
        UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                client[idx]
                , strlen(test_string3) + 1
                , test_string3
                , recv_bufs_size
                , (void *)recv_bufs
                , 10000))));
        UNITTEST_EXPECT_EQ(strlen(response_string3) + 1, recv_count);
        UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string3));
        UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                client[idx]
                , strlen(test_string4) + 1
                , test_string4
                , recv_bufs_size
                , (void *)recv_bufs
                , 10000*60))));
        UNITTEST_EXPECT_EQ(strlen(response_string4) + 1, recv_count);
        UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string4));
    }
    cplus_systime_sleep_msec(5 * 1000);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, client_count);
    for (int32_t idx = 0; idx < MAX_CLIENT_COUNT; idx++)
    {
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[idx]));
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_server_delete(ipc_server));
    UNITTEST_EXPECT_EQ(0, client_count);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[0]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[1]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[2]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[3]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[4]);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

int32_t serv_oneway_on_received(
    cplus_socket conn_sock
    , uint32_t input_bufs_len
    , void * input_bufs
    , uint32_t * output_bufs_len
    , void * output_bufs)
{
    if (((strlen(test_string0) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string0)))
    {
        verification_count[0] += 1;
    }
    else if (((strlen(test_string1) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string1)))
    {
        verification_count[1] += 1;
    }
    else if (((strlen(test_string2) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string2)))
    {
        verification_count[2] += 1;
    }
    else if (((strlen(test_string3) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string3)))
    {
        verification_count[3] += 1;
    }
    else if (((strlen(test_string4) + 1) == input_bufs_len)
        and (0 == strcmp(input_bufs, test_string4)))
    {
        verification_count[4] += 1;
    }
    return CPLUS_SUCCESS;
}

CPLUS_UNIT_TEST(CPLUS_IPC_CLIENT_SEND_ONEWAY, functionity)
{
    client_count = 0;
    for (int i = 0; i < sizeof(verification_count)/sizeof(uint32_t); i++)
    {
        verification_count[i] = 0;
    }

    cplus_ipc_server ipc_server = NULL;
    cplus_ipc_client client[MAX_CLIENT_COUNT] = {0};

    CPLUS_IPC_CB_FUNCS_T ipc_server_cb_funcs = {0};
    ipc_server_cb_funcs.on_connected = serv_on_connected;
    ipc_server_cb_funcs.on_received = serv_oneway_on_received;
    ipc_server_cb_funcs.on_disconnected = serv_on_disconnected;

    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_server = cplus_ipc_server_new(SERVER_NAME, MAX_CLIENT_COUNT, &ipc_server_cb_funcs))));
    for (int32_t idx = 0; idx < MAX_CLIENT_COUNT; idx++)
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (client[idx] = cplus_ipc_client_new(SERVER_NAME, NULL))));
        UNITTEST_EXPECT_EQ(strlen(test_string0) + 1, cplus_ipc_client_send_oneway(client[idx], strlen(test_string0) + 1, test_string0));
        UNITTEST_EXPECT_EQ(strlen(test_string1) + 1, cplus_ipc_client_send_oneway(client[idx], strlen(test_string1) + 1, test_string1));
        UNITTEST_EXPECT_EQ(strlen(test_string2) + 1, cplus_ipc_client_send_oneway(client[idx], strlen(test_string2) + 1, test_string2));
        UNITTEST_EXPECT_EQ(strlen(test_string3) + 1, cplus_ipc_client_send_oneway(client[idx], strlen(test_string3) + 1, test_string3));
        UNITTEST_EXPECT_EQ(strlen(test_string4) + 1, cplus_ipc_client_send_oneway(client[idx], strlen(test_string4) + 1, test_string4));
    }
    cplus_systime_sleep_msec(5 * 1000);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, client_count);
    for (int32_t idx = 0; idx < MAX_CLIENT_COUNT; idx++)
    {
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[idx]));
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_server_delete(ipc_server));
    UNITTEST_EXPECT_EQ(0, client_count);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[0]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[1]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[2]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[3]);
    UNITTEST_EXPECT_EQ(MAX_CLIENT_COUNT, verification_count[4]);
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_IPC_CLIENT_CONNECT, bad_case_over_connection_count)
{
    cplus_ipc_server ipc_server = NULL;
    cplus_ipc_client client[MAX_CLIENT_COUNT + 1] = {0};

    CPLUS_IPC_CB_FUNCS_T ipc_server_cb_funcs = {0};
    ipc_server_cb_funcs.on_connected = serv_on_connected;
    ipc_server_cb_funcs.on_received = serv_oneway_on_received;
    ipc_server_cb_funcs.on_disconnected = serv_on_disconnected;

    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_server = cplus_ipc_server_new(SERVER_NAME, MAX_CLIENT_COUNT, &ipc_server_cb_funcs))));
    for (int32_t i = 0; i < MAX_CLIENT_COUNT; i++)
    {
        UNITTEST_EXPECT_EQ(true, (NULL != (client[i] = cplus_ipc_client_new(SERVER_NAME, NULL))));
    }
    UNITTEST_EXPECT_EQ(true, (NULL == (client[5] = cplus_ipc_client_new(SERVER_NAME, NULL))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[0]));
    UNITTEST_EXPECT_EQ(true, (NULL != (client[5] = cplus_ipc_client_new(SERVER_NAME, NULL))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_send_heartbeat(client[5], 3000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_send_heartbeat(client[5], 3000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[2]));
    UNITTEST_EXPECT_EQ(true, (NULL != (client[2] = cplus_ipc_client_new(SERVER_NAME, NULL))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_send_heartbeat(client[2], 3000));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[1]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[2]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[3]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[4]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(client[5]));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_server_delete(ipc_server));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_IPC_CLIENT_SEND_REQUEST, bad_case_timeout)
{
    cplus_ipc_server ipc_server = NULL;
    cplus_ipc_client ipc_client = NULL;

    char recv_bufs[512] = {0};
    uint32_t recv_bufs_size = sizeof(recv_bufs);
    int32_t recv_count = 0;

    CPLUS_IPC_CB_FUNCS_T ipc_server_cb_funcs = {0};
    ipc_server_cb_funcs.on_connected = serv_on_connected;
    ipc_server_cb_funcs.on_received = serv_request_on_received;
    ipc_server_cb_funcs.on_disconnected = serv_on_disconnected;

    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_server = cplus_ipc_server_new(SERVER_NAME, MAX_CLIENT_COUNT, &ipc_server_cb_funcs))));
    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_client = cplus_ipc_client_new(SERVER_NAME, NULL))));
    UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                ipc_client
                , strlen(test_string0) + 1
                , test_string0
                , recv_bufs_size
                , (void *)recv_bufs
                , 1000))));
    UNITTEST_EXPECT_EQ(strlen(response_string0) + 1, recv_count);
    UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string0));
    UNITTEST_EXPECT_NE(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                ipc_client
                , strlen(test_string1) + 1
                , test_string1
                , recv_bufs_size
                , (void *)recv_bufs
                , 1))));
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    UNITTEST_EXPECT_EQ(
            true
            , (0 < (recv_count = cplus_ipc_client_send_request(
                ipc_client
                , strlen(test_string2) + 1
                , test_string2
                , recv_bufs_size
                , (void *)recv_bufs
                , 1000))));
    UNITTEST_EXPECT_EQ(strlen(response_string2) + 1, recv_count);
    UNITTEST_EXPECT_EQ(0, strcmp(recv_bufs, response_string2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(ipc_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_server_delete(ipc_server));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_IPC_CLIENT_SEND_HEARTBEAT, bad_case_timeout)
{
    cplus_ipc_server ipc_server = NULL;
    cplus_ipc_client ipc_client = NULL;

    CPLUS_IPC_CB_FUNCS_T ipc_server_cb_funcs = {0};
    ipc_server_cb_funcs.on_connected = serv_on_connected;
    ipc_server_cb_funcs.on_received = serv_oneway_on_received;
    ipc_server_cb_funcs.on_disconnected = serv_on_disconnected;

    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_server = cplus_ipc_server_new(SERVER_NAME, MAX_CLIENT_COUNT, &ipc_server_cb_funcs))));
    UNITTEST_EXPECT_EQ(true, (NULL != (ipc_client = cplus_ipc_client_new(SERVER_NAME, NULL))));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_ipc_client_send_heartbeat(ipc_client, 1));
    UNITTEST_EXPECT_EQ(ETIMEDOUT, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_client_delete(ipc_client));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_ipc_server_delete(ipc_server));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_ipc_server(void)
{
    UNITTEST_ADD_TESTCASE(CPLUS_IPC_CLIENT_SEND_REQUEST, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_IPC_CLIENT_SEND_ONEWAY, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_IPC_CLIENT_CONNECT, bad_case_over_connection_count);
    UNITTEST_ADD_TESTCASE(CPLUS_IPC_CLIENT_SEND_REQUEST, bad_case_timeout);
    UNITTEST_ADD_TESTCASE(CPLUS_IPC_CLIENT_SEND_HEARTBEAT, bad_case_timeout);
}

#endif // __CPLUS_UNITTEST__
