#ifndef __CPLUS_IPC_SERVER_H__
#define __CPLUS_IPC_SERVER_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (* CPLUS_IPC_CB_ON_CONNECTED)(cplus_socket conn_sock);
typedef int32_t (* CPLUS_IPC_CB_ON_ERROR)(cplus_socket conn_sock, int32_t sock_errno);
typedef int32_t (* CPLUS_IPC_CB_ON_DISCONNECTED)(cplus_socket conn_sock);
typedef int32_t (* CPLUS_IPC_CB_ON_RECEIVED)\
    (cplus_socket conn_sock, uint32_t input_bufs_len, void * input_bufs, uint32_t * output_bufs_len, void * output_bufs);

typedef struct cplus_ipc_cb_funcs
{
    CPLUS_IPC_CB_ON_CONNECTED on_connected;
    CPLUS_IPC_CB_ON_ERROR on_error;
    CPLUS_IPC_CB_ON_DISCONNECTED on_disconnected;
    CPLUS_IPC_CB_ON_RECEIVED on_received;
} *CPLUS_IPC_CB_FUNCS, CPLUS_IPC_CB_FUNCS_T;

cplus_ipc_server cplus_ipc_server_new(const char * name, uint32_t max_connection, CPLUS_IPC_CB_FUNCS cb_funcs);
cplus_ipc_client cplus_ipc_client_new(const char * name, CPLUS_IPC_CB_FUNCS cb_funcs);
int32_t cplus_ipc_server_delete(cplus_ipc_server obj);
int32_t cplus_ipc_client_delete(cplus_ipc_client obj);
bool cplus_ipc_server_check(cplus_object obj);
bool cplus_ipc_client_check(cplus_object obj);
int32_t cplus_ipc_client_send_request(cplus_ipc_client obj, uint32_t input_bufs_len, void * input_bufs
    , uint32_t output_bufs_len, void * output_bufs, uint32_t timeout);
int32_t cplus_ipc_client_send_oneway(cplus_ipc_client obj, uint32_t input_bufs_len, void * input_bufs);
int32_t cplus_ipc_client_send_heartbeat(cplus_ipc_client obj, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_IPC_SERVER_H__