#ifndef __CPLUS_MUTEX_H__
#define __CPLUS_MUTEX_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

cplus_mutex cplus_mutex_new(void);
cplus_mutex cplus_mutex_new_xp(const char * name);
cplus_mutex cplus_mutex_create_xp(const char * name);
cplus_mutex cplus_mutex_open_xp(const char * name);
int32_t cplus_mutex_delete(cplus_mutex obj);

bool cplus_mutex_check(cplus_object obj);
int32_t cplus_mutex_lock(cplus_mutex obj, uint32_t timeout);
int32_t cplus_mutex_unlock(cplus_mutex obj);

int32_t cplus_crit_sect_enter(cplus_mutex obj);
int32_t cplus_crit_sect_exit(cplus_mutex obj);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_MUTEX_H__