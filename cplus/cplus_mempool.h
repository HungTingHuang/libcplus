#ifndef __CPLUS_MEMPOOL_H__
#define __CPLUS_MEMPOOL_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

cplus_mempool cplus_mempool_new(uint32_t block_count, uint32_t block_size);
cplus_mempool cplus_mempool_new_s(uint32_t block_count, uint32_t block_size);
int32_t cplus_mempool_delete(cplus_mempool obj);

bool cplus_mempool_check(cplus_object obj);

uint32_t cplus_mempool_get_free_blocks_count(cplus_mempool obj);

void * cplus_mempool_alloc(cplus_mempool obj);
int32_t cplus_mempool_free(cplus_mempool obj, void * mem);

uint32_t cplus_mempool_get_index(cplus_mempool obj, void * addr);
void * cplus_mempool_get_addr_by_index(cplus_mempool obj, uint32_t index);
uint32_t cplus_mempool_alloc_as_index(cplus_mempool obj);
int32_t cplus_mempool_free_by_index(cplus_mempool obj, uint32_t index);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_MEMPOOL_H__