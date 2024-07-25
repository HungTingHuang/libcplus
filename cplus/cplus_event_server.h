#ifndef __CPLUS_EVENT_SERVER_H__
#define __CPLUS_EVENT_SERVER_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef int32_t (* CPLUS_EVENT_SERVER_CB_ON_ERROR)(int32_t fd, int32_t error_no, void * cb_param);
typedef int32_t (* CPLUS_EVENT_SERVER_CB_ON_TIMEOUT)(int32_t fd, void * cb_param);
typedef int32_t (* CPLUS_EVENT_SERVER_CB_ON_READ)(int32_t fd, int32_t value, void * cb_param);

typedef struct cplus_event_server_cb_funcs
{
    CPLUS_EVENT_SERVER_CB_ON_ERROR on_error;
    CPLUS_EVENT_SERVER_CB_ON_TIMEOUT on_timeout;
    CPLUS_EVENT_SERVER_CB_ON_READ on_read;
} *CPLUS_EVENT_SERVER_CB_FUNCS, CPLUS_EVENT_SERVER_CB_FUNCS_T;

typedef enum cplus_event_server_flag
{
    CPLUS_EVENT_SERVER_FLAG_NONE        = 0,
    CPLUS_EVENT_SERVER_FLAG_CLOEXEC     = 1,
    CPLUS_EVENT_SERVER_FLAG_NONBLOCK    = 1 << 1,
    CPLUS_EVENT_SERVER_FLAG_SYNC        = 1 << 2,
} CPLUS_EVENT_SERVER_FLAG;

typedef struct cplus_event_server_config
{
    bool enable_ipc;
    uint32_t init_val;
    CPLUS_EVENT_SERVER_FLAG flag;
    bool start;
    void * cb_param;
} *CPLUS_EVENT_SERVER_CONFIG, CPLUS_EVENT_SERVER_CONFIG_T;

cplus_event_server cplus_event_server_new(
    uint32_t init_val
    , CPLUS_EVENT_SERVER_FLAG flag
    , CPLUS_EVENT_SERVER_CB_FUNCS cb_funcs);

cplus_event_server cplus_event_server_new_config(
    CPLUS_EVENT_SERVER_CONFIG config
    , CPLUS_EVENT_SERVER_CB_FUNCS cb_funcs);

cplus_event_client cplus_event_client_attach(cplus_event_server obj);
cplus_event_client cplus_event_client_connect(void);

bool cplus_event_server_check(cplus_object obj);
bool cplus_event_client_check(cplus_object obj);

int32_t cplus_event_server_delete(cplus_event_server obj);
int32_t cplus_event_client_delete(cplus_event_client obj);

int32_t cplus_event_server_start(cplus_event_server obj);
int32_t cplus_event_server_get_efd(cplus_event_server obj);

int32_t cplus_event_client_add_count(cplus_event_client obj, uint64_t count);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_EVENT_SERVER_H__