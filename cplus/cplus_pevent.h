#ifndef __CPLUS_PEVENT_H__
#define __CPLUS_PEVENT_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

cplus_pevent cplus_pevent_new(bool broadcast, bool status);
cplus_pevent cplus_pevent_new_xp(const char * name, bool broadcast, bool status);
cplus_pevent cplus_pevent_create_xp(const char * name, bool broadcast, bool status);
cplus_pevent cplus_pevent_open_xp(const char * name);
int32_t cplus_pevent_delete(cplus_pevent obj);

bool cplus_pevent_check(cplus_object obj);
int32_t cplus_pevent_set(cplus_pevent obj);
int32_t cplus_pevent_wait(cplus_pevent obj, uint32_t timeout);
int32_t cplus_pevent_reset(cplus_pevent obj);
bool cplus_pevent_get_status(cplus_pevent obj);

#ifdef __cplusplus
}
#endif

#endif //__CPLUS_PEVENT_H__