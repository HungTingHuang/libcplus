#ifndef __CPLUS_MEMMGR_H__
#define __CPLUS_MEMMGR_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __CPLUS_MEM_MANAGER__
#define cplus_malloc(size) cplus_mgr_malloc(size, __FILE__, __FUNCTION__, __LINE__)
#define cplus_realloc(ptr, size) cplus_mgr_realloc(ptr, size, __FILE__, __FUNCTION__, __LINE__)
void * cplus_mgr_malloc(
    uint32_t size
    , const char * file
    , const char * function
    , uint32_t line);
void * cplus_mgr_realloc(
    void * ptr
    , uint32_t size
    , const char * file
    , const char * function
    , uint32_t line);
#else // else __CPLUS_MEM_MANAGER__
#define cplus_malloc(size) cplus_mgr_malloc(size)
#define cplus_realloc(ptr, size) cplus_mgr_realloc(ptr, size)
void * cplus_mgr_malloc(uint32_t size);
void * cplus_mgr_realloc(void * ptr, uint32_t size);
#endif // __CPLUS_MEM_MANAGER__
#define cplus_free(addr) ({ int32_t res = cplus_mgr_free(addr); addr = NULL; res; })
int32_t cplus_mgr_report(void);
int32_t cplus_mgr_report_in_file(void * stream);
int32_t cplus_mgr_check_size(void * ptr, uint32_t size);
int32_t cplus_mgr_check_boundary(void * ptr);
int32_t cplus_mgr_free(void * addr);
void * cplus_mem_cpy(void * dest, void * src, uint32_t count);
void * cplus_mem_cpy_ex(void * dest, uint32_t destsz, void * src, uint32_t count);
void * cplus_mem_set(void * dest, uint8_t value, uint32_t count);
int32_t cplus_str_printf(void * str, uint32_t size, const char * format, ...);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_MEMMGR_H__