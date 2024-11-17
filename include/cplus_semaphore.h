#ifndef __CPLUS_SEMAPHORE_H__
#define __CPLUS_SEMAPHORE_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

cplus_semaphore cplus_semaphore_new(int32_t initial_count);
cplus_semaphore cplus_semaphore_new_xp(const char * name, int32_t initial_count);
cplus_semaphore cplus_semaphore_create_xp(const char * name, int32_t initial_count);
cplus_semaphore cplus_semaphore_open_xp(const char * name);
bool cplus_semaphore_check(cplus_object obj);
int32_t cplus_semaphore_delete(cplus_semaphore obj);
int32_t cplus_semaphore_wait_poll(cplus_semaphore obj, uint32_t timeout);
int32_t cplus_semaphore_push(cplus_semaphore obj, int32_t count);
int32_t cplus_semaphore_get_value(cplus_semaphore obj);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_SEMAPHORE_H__

