/******************************************************************
* @file: cplus.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/
#include "cplus.h"
#include "common.h"

int32_t cplus_object_delete(cplus_object object)
{
    CHECK_NOT_NULL(object, CPLUS_FAIL);

    if (cplus_mempool_check(object))
    {
        return cplus_mempool_delete(object);
    }
    else if (cplus_sharedmem_check(object))
    {
        return cplus_sharedmem_delete(object);
    }
    else if (cplus_task_check(object))
    {
        return cplus_task_delete(object);
    }
    else if (cplus_taskpool_check(object))
    {
        return cplus_taskpool_delete(object);
    }
    else if (cplus_syslog_check(object))
    {
        return cplus_syslog_delete(object);
    }
    else if (cplus_syslog_check(object))
    {
        return cplus_syslog_delete(object);
    }
    else if (cplus_data_check(object))
    {
        return cplus_data_delete(object);
    }
    else if (cplus_llist_check(object))
    {
        return cplus_llist_delete(object);
    }
    else if (cplus_pevent_check(object))
    {
        return cplus_pevent_delete(object);
    }
    else if (cplus_rwlock_check(object))
    {
        return cplus_rwlock_delete(object);
    }
    else if (cplus_semaphore_check(object))
    {
        return cplus_semaphore_delete(object);
    }
    else if (cplus_mutex_check(object))
    {
        return cplus_mutex_delete(object);
    }
    else if (cplus_socket_check(object))
    {
        return cplus_socket_delete(object);
    }
    else if (cplus_ipc_server_check(object))
    {
        return cplus_ipc_server_delete(object);
    }
    else if (cplus_ipc_client_check(object))
    {
        return cplus_ipc_client_delete(object);
    }
    else if (cplus_file_check(object))
    {
        return cplus_file_delete(object);
    }
    return CPLUS_FAIL;
}

int32_t cplus_lock_shlock(cplus_object object, uint32_t timeout)
{
    if (object)
    {
        if (cplus_rwlock_check(object))
        {
            return cplus_rwlock_rdlock(object, timeout);
        }
        else if (cplus_mutex_check(object))
        {
            return cplus_mutex_lock(object, timeout);
        }
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_lock_exlock(cplus_object object, uint32_t timeout)
{
    if (object)
    {
        if (cplus_rwlock_check(object))
        {
            return cplus_rwlock_wrlock(object, timeout);
        }
        else if (cplus_mutex_check(object))
        {
            return cplus_mutex_lock(object, timeout);
        }
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_lock_unlock(cplus_object object)
{
    if (object)
    {
        if (cplus_rwlock_check(object))
        {
            return cplus_rwlock_unlock(object);
        }
        else if (cplus_mutex_check(object))
        {
            return cplus_mutex_unlock(object);
        }
    }
    return CPLUS_SUCCESS;
}


