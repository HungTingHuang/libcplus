#ifndef __CPLUS_LLIST_H__
#define __CPLUS_LLIST_H__
#include "cplus_typedef.h"
#include "cplus_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPLUS_LLIST_FOREACH(LLIST, ITERATOR) \
    for (uint32_t INDEX = 0 \
        ; CPLUS_NULL != LLIST && INDEX < cplus_llist_get_size(LLIST) \
        && (CPLUS_NULL != (ITERATOR = (typeof(ITERATOR))cplus_llist_get_of(LLIST, INDEX))) \
        ; INDEX++)

cplus_llist cplus_llist_new(void);
cplus_llist cplus_llist_prev_new(uint32_t max_count);
cplus_llist cplus_llist_new_s(void);
cplus_llist cplus_llist_prev_new_s(uint32_t max_count);
int32_t cplus_llist_delete(cplus_llist obj);
int32_t cplus_llist_clear(cplus_llist obj);
bool cplus_llist_check(cplus_object obj);
uint32_t cplus_llist_get_size(cplus_llist obj);
int32_t cplus_llist_push_at(cplus_llist obj, int32_t index, void * data);
void * cplus_llist_pop_of(cplus_llist obj, int32_t index);
int32_t cplus_llist_push_back(cplus_llist obj, void * data);
int32_t cplus_llist_push_front(cplus_llist obj, void * data);
void * cplus_llist_pop_back(cplus_llist obj);
void * cplus_llist_pop_front(cplus_llist obj);
void * cplus_llist_get_of(cplus_llist obj, int32_t index);
void * cplus_llist_get_head(cplus_llist obj);
void * cplus_llist_get_tail(cplus_llist obj);
void * cplus_llist_get_current(cplus_llist obj);
void * cplus_llist_get_next(cplus_llist obj);
void * cplus_llist_get_prev(cplus_llist obj);
void * cplus_llist_get_cycling_next(cplus_llist obj);
bool cplus_llist_is_sort(cplus_llist obj);
int32_t cplus_llist_sort(cplus_llist obj, int32_t (* comparator)(void * data1, void * data2));
int32_t cplus_llist_get_index_if(cplus_llist obj, int32_t (* comparator)(void * data, void * arg), void * arg);
cplus_llist cplus_llist_get_set_if(cplus_llist obj, int32_t (* comparator)(void * data, void * arg), void * arg);
void * cplus_llist_get_if(cplus_llist obj, int32_t (* comparator)(void * data, void * arg), void * arg);
void * cplus_llist_pop_if(cplus_llist obj, int32_t (* comparator)(void * data, void * arg), void * arg);
bool cplus_llist_is_consist(cplus_llist obj1, cplus_llist obj2, int32_t (* comparator)(void * data1, void * data2));
int32_t cplus_llist_delete_data(cplus_llist obj, void * data);
cplus_data cplus_llist_add_data(cplus_llist obj, CPLUS_DATA_TYPE type, const char * key, void * value1, void * value2);
cplus_data cplus_llist_find_data(cplus_llist obj, const char * key);
cplus_data cplus_llist_update_data(cplus_llist obj, const char * key, cplus_data data);
int32_t cplus_llist_remove_data(cplus_llist obj, const char * key);
cplus_data cplus_llist_add_data_bool(cplus_llist obj, const char * key, bool value);
cplus_data cplus_llist_add_data_int8(cplus_llist obj, const char * key, int8_t value);
cplus_data cplus_llist_add_data_int16(cplus_llist obj, const char * key, int16_t value);
cplus_data cplus_llist_add_data_int32(cplus_llist obj, const char * key, int32_t value);
cplus_data cplus_llist_add_data_int64(cplus_llist obj, const char * key, int64_t value);
cplus_data cplus_llist_add_data_uint8(cplus_llist obj, const char * key, uint8_t value);
cplus_data cplus_llist_add_data_uint16(cplus_llist obj, const char * key, uint16_t value);
cplus_data cplus_llist_add_data_uint32(cplus_llist obj, const char * key, uint32_t value);
cplus_data cplus_llist_add_data_uint64(cplus_llist obj, const char * key, uint64_t value);
cplus_data cplus_llist_add_data_float(cplus_llist obj, const char * key, float value);
cplus_data cplus_llist_add_data_double(cplus_llist obj, const char * key, double value);
cplus_data cplus_llist_add_data_pointer(cplus_llist obj, const char * key, void * value);
cplus_data cplus_llist_add_data_string(cplus_llist obj, const char * key, char * str_bufs, uint32_t str_len);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_LLIST_H__