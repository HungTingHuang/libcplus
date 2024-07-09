#ifndef __CPLUS_RWLOCK_H__
#define __CPLUS_RWLOCK_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cplus_rwlock_mode
{
    CPLUS_RWLOCK_MODE_NONE = 0,
    CPLUS_RWLOCK_MODE_CREATE,
    CPLUS_RWLOCK_MODE_OPEN,
    CPLUS_RWLOCK_MODE_HYBRID,
    CPLUS_RWLOCK_MODE_MAX,
} CPLUS_RWLOCK_MODE;

typedef enum cplus_rwlock_kind
{
    CPLUS_RWLOCK_KIND_NONE = 0,
    CPLUS_RWLOCK_KIND_PREFER_READER,
    CPLUS_RWLOCK_KIND_PREFER_WRITER,
    CPLUS_RWLOCK_KIND_PREFER_WRITER_NONRECURSIVE,
    CPLUS_RWLOCK_KIND_MAX,
} CPLUS_RWLOCK_KIND;

typedef struct cplus_rwlock_config
{
    char * name;
    CPLUS_RWLOCK_MODE mode;
    CPLUS_RWLOCK_KIND kind;
} *CPLUS_RWLOCK_CONFIG, CPLUS_RWLOCK_CONFIG_T;

cplus_rwlock cplus_rwlock_new_xp(const char * name);
cplus_rwlock cplus_rwlock_create_xp(const char * name);
cplus_rwlock cplus_rwlock_open_xp(const char * name);
cplus_rwlock cplus_rwlock_new_config(CPLUS_RWLOCK_CONFIG config);
cplus_rwlock cplus_rwlock_new(void);
int32_t cplus_rwlock_delete(cplus_rwlock obj);

bool cplus_rwlock_check(cplus_object obj);
int32_t cplus_rwlock_rdlock(cplus_rwlock obj, uint32_t timeout);
int32_t cplus_rwlock_wrlock(cplus_rwlock obj, uint32_t timeout);
int32_t cplus_rwlock_unlock(cplus_rwlock obj);

#ifdef __cplusplus
}
#endif

#endif //__CPLUS_RWLOCK_H__