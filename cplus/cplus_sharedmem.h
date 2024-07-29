#ifndef __CPLUS_SHAREDMEM_H__
#define __CPLUS_SHAREDMEM_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

cplus_sharedmem cplus_sharedmem_new(const char * name, uint32_t size);
cplus_sharedmem cplus_sharedmem_create(const char * name, uint32_t size);
cplus_sharedmem cplus_sharedmem_open(const char * name);
int32_t cplus_sharedmem_delete(cplus_sharedmem obj);
bool cplus_sharedmem_check(cplus_object obj);
void * cplus_sharedmem_alloc(cplus_sharedmem obj);
int32_t cplus_sharedmem_get_size(cplus_sharedmem obj);
char * cplus_sharedmem_get_name(cplus_sharedmem obj);
bool cplus_sharedmem_is_owner(cplus_sharedmem obj);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_SHAREDMEM_H__