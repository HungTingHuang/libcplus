#ifndef __CPLUS_DATA_H__
#define __CPLUS_DATA_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPLUS_DATA_REVERSE(DATAPTR) \
    do { \
        switch (sizeof(typeof((* (DATAPTR))))) \
        { \
        default: \
            break; \
        case 2: \
            cplus_data_reverse16((uint16_t *)(DATAPTR)); \
            break; \
        case 4: \
            cplus_data_reverse32((uint32_t *)(DATAPTR)); \
            break; \
        case 8: \
            cplus_data_reverse64((uint64_t *)(DATAPTR)); \
            break; \
        } \
    } while (0)

#define CPLUS_DATA_SWAP(DATA) \
    ({ \
        typeof(DATA) value = 0; \
        switch (sizeof(typeof(DATA))) \
        { \
        default: \
            value = CPLUS_FAIL; \
            break; \
        case 2: \
            value = cplus_data_swap16((uint16_t)DATA); \
            break; \
        case 4: \
            value = cplus_data_swap32((uint32_t)DATA); \
            break; \
        case 8: \
            value = cplus_data_swap64((uint64_t)DATA); \
            break; \
        } \
        value; \
    })

#ifdef __cplusplus
    #define CPLUS_DATA_TYPE_CONV(DATA) CPLUS_DATA_TYPE_UNKNOWN
#else
    #define CPLUS_DATA_TYPE_CONV(DATA) _Generic((DATA), \
        bool: CPLUS_DATA_TYPE_BOOL, \
        int8_t: CPLUS_DATA_TYPE_INT8, \
        int16_t: CPLUS_DATA_TYPE_INT16, \
        int32_t: CPLUS_DATA_TYPE_INT32, \
        int64_t: CPLUS_DATA_TYPE_INT64, \
        uint8_t: CPLUS_DATA_TYPE_UINT8, \
        uint16_t: CPLUS_DATA_TYPE_UINT16, \
        uint32_t: CPLUS_DATA_TYPE_UINT32, \
        uint64_t: CPLUS_DATA_TYPE_UINT64, \
        float: CPLUS_DATA_TYPE_FLOAT, \
        double: CPLUS_DATA_TYPE_DOUBLE, \
        void *: CPLUS_DATA_TYPE_POINTER, \
        char *: CPLUS_DATA_TYPE_STRING, \
        default: CPLUS_DATA_TYPE_UNKNOWN)
#endif

typedef enum cplus_data_type
{
    CPLUS_DATA_TYPE_NULL        = 0,
    CPLUS_DATA_TYPE_BOOL        = 1,
    CPLUS_DATA_TYPE_INT8        = 2,
    CPLUS_DATA_TYPE_INT16       = 3,
    CPLUS_DATA_TYPE_INT32       = 4,
    CPLUS_DATA_TYPE_INT64       = 5,
    CPLUS_DATA_TYPE_UINT8       = 6,
    CPLUS_DATA_TYPE_UINT16      = 7,
    CPLUS_DATA_TYPE_UINT32      = 8,
    CPLUS_DATA_TYPE_UINT64      = 9,
    CPLUS_DATA_TYPE_FLOAT       = 10,
    CPLUS_DATA_TYPE_DOUBLE      = 11,
    CPLUS_DATA_TYPE_POINTER     = 12,
    CPLUS_DATA_TYPE_STRING      = 13,
    CPLUS_DATA_TYPE_BYTE_ARRAY  = 14,
    CPLUS_DATA_TYPE_UNKNOWN     = 15,
} CPLUS_DATA_TYPE;

typedef union cplus_data_value
{
    bool        b;
    int8_t      i8;
    int16_t     i16;
    int32_t     i32;
    int64_t     i64;
    uint8_t     u8;
    uint16_t    u16;
    uint32_t    u32;
    uint64_t    u64;
    float       f;
    double      db;
    void *      p;
    struct
    {
        uint32_t len;
        char * bufs;
    } str;
    struct
    {
        uint32_t len;
        uint8_t * bufs;
    } byte_array;
} CPLUS_DATA_VALUE;

cplus_data cplus_data_new(CPLUS_DATA_TYPE type, void * value1, void * value2);
cplus_data cplus_data_new_bool(bool value);
cplus_data cplus_data_new_int8(int8_t value);
cplus_data cplus_data_new_int16(int16_t value);
cplus_data cplus_data_new_int32(int32_t value);
cplus_data cplus_data_new_int64(int64_t value);
cplus_data cplus_data_new_uint8(uint8_t value);
cplus_data cplus_data_new_uint16(uint16_t value);
cplus_data cplus_data_new_uint32(uint32_t value);
cplus_data cplus_data_new_uint64(uint64_t value);
cplus_data cplus_data_new_float(float value);
cplus_data cplus_data_new_double(double value);
cplus_data cplus_data_new_pointer(void * value);
cplus_data cplus_data_new_string(uint32_t str_len, char * str_bufs);
cplus_data cplus_data_new_byte_array(uint32_t array_len, uint8_t * array_bufs);
cplus_data cplus_data_new_ex(CPLUS_DATA_TYPE type, void * value1, void * value2, uint32_t key_len, const char * key);
cplus_data cplus_data_new_bool_ex(bool value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_int8_ex(int8_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_int16_ex(int16_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_int32_ex(int32_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_int64_ex(int64_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_uint8_ex(uint8_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_uint16_ex(uint16_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_uint32_ex(uint32_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_uint64_ex(uint64_t value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_float_ex(float value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_double_ex(double value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_pointer_ex(void * value, uint32_t key_len, const char * key);
cplus_data cplus_data_new_string_ex(uint32_t str_len, char * str_bufs, uint32_t key_len, const char * key);
cplus_data cplus_data_new_byte_array_ex(uint32_t array_len, uint8_t * array_bufs, uint32_t key_len, const char * key);
int32_t cplus_data_delete(cplus_data obj);
bool cplus_data_check(cplus_object obj);
int32_t cplus_data_get_type(cplus_data obj);
char * cplus_data_get_type_str(CPLUS_DATA_TYPE type);
uint32_t cplus_data_get_data_size(cplus_data obj);
int32_t cplus_data_get_value(cplus_data obj, void * value1, void * value2);
int32_t cplus_data_set_value(cplus_data obj, void * value1, void * value2);
bool cplus_data_is_valid(cplus_data obj);
const char * cplus_data_get_key(cplus_data obj);
int32_t cplus_data_set_key(cplus_data obj, uint32_t key_len, const char * key);
int32_t cplus_data_clone_value(cplus_data dest, cplus_data src);
uint16_t cplus_data_swap16(uint16_t value);
uint32_t cplus_data_swap32(uint32_t value);
uint64_t cplus_data_swap64(uint64_t value);
void cplus_data_reverse16(uint16_t * value);
void cplus_data_reverse32(uint32_t * value);
void cplus_data_reverse64(uint64_t * value);
int32_t cplus_data_set_action_mode(cplus_data obj, uint32_t mode);
uint32_t cplus_data_get_action_mode(cplus_data obj);
int32_t cplus_data_set_bool(cplus_data obj, bool value);
int32_t cplus_data_set_int8(cplus_data obj, int8_t value);
int32_t cplus_data_set_int16(cplus_data obj, int16_t value);
int32_t cplus_data_set_int32(cplus_data obj, int32_t value);
int32_t cplus_data_set_int64(cplus_data obj, int64_t value);
int32_t cplus_data_set_uint8(cplus_data obj, uint8_t value);
int32_t cplus_data_set_uint16(cplus_data obj, uint16_t value);
int32_t cplus_data_set_uint32(cplus_data obj, uint32_t value);
int32_t cplus_data_set_uint64(cplus_data obj, uint64_t value);
int32_t cplus_data_set_float(cplus_data obj, float value);
int32_t cplus_data_set_double(cplus_data obj, double value);
int32_t cplus_data_set_pointer(cplus_data obj, void * value);
int32_t cplus_data_set_string(cplus_data obj, uint32_t str_len, char * str_bufs);
int32_t cplus_data_set_byte_array(cplus_data obj, uint32_t array_len, uint8_t * array_bufs);
int32_t cplus_data_set_string_code(cplus_data obj, uint32_t code);
bool cplus_data_get_bool(cplus_data obj);
int8_t cplus_data_get_int8(cplus_data obj);
int16_t cplus_data_get_int16(cplus_data obj);
int32_t cplus_data_get_int32(cplus_data obj);
int64_t cplus_data_get_int64(cplus_data obj);
uint8_t cplus_data_get_uint8(cplus_data obj);
uint16_t cplus_data_get_uint16(cplus_data obj);
uint32_t cplus_data_get_uint32(cplus_data obj);
uint64_t cplus_data_get_uint64(cplus_data obj);
float cplus_data_get_float(cplus_data obj);
double cplus_data_get_double(cplus_data obj);
void * cplus_data_get_pointer(cplus_data obj);
uint8_t * cplus_data_get_byte_array(cplus_data obj);
char * cplus_data_get_string(cplus_data obj);
uint32_t cplus_data_get_string_code(cplus_data obj);
cplus_data cplus_data_create_group_node(char * data_group_name);
cplus_llist cplus_data_get_group(cplus_data data_group_node);
int32_t cplus_data_delete_group_node(cplus_data data_group_node);
int32_t cplus_data_get_as_bool(cplus_data obj, bool * value);
int32_t cplus_data_get_as_int8(cplus_data obj, int8_t * value);
int32_t cplus_data_get_as_int16(cplus_data obj, int16_t * value);
int32_t cplus_data_get_as_int32(cplus_data obj, int32_t * value);
int32_t cplus_data_get_as_int64(cplus_data obj, int64_t * value);
int32_t cplus_data_get_as_uint8(cplus_data obj, uint8_t * value);
int32_t cplus_data_get_as_uint16(cplus_data obj, uint16_t * value);
int32_t cplus_data_get_as_uint32(cplus_data obj, uint32_t * value);
int32_t cplus_data_get_as_uint64(cplus_data obj, uint64_t * value);
int32_t cplus_data_get_as_float(cplus_data obj, float * value);
int32_t cplus_data_get_as_double(cplus_data obj, double * value);
/* int32_t cplus_data_get_as_pointer(cplus_data obj, void * value); */
int32_t cplus_data_get_as_byte_array(cplus_data obj, uint32_t array_len, uint8_t * array_bufs);
int32_t cplus_data_get_as_string(cplus_data obj, uint32_t str_len, char * str_bufs);
int32_t cplus_data_set_as_bool(cplus_data obj, bool value);
int32_t cplus_data_set_as_int8(cplus_data obj, int8_t value);
int32_t cplus_data_set_as_int16(cplus_data obj, int16_t value);
int32_t cplus_data_set_as_int32(cplus_data obj, int32_t value);
int32_t cplus_data_set_as_int64(cplus_data obj, int64_t value);
int32_t cplus_data_set_as_uint8(cplus_data obj, uint8_t value);
int32_t cplus_data_set_as_uint16(cplus_data obj, uint16_t value);
int32_t cplus_data_set_as_uint32(cplus_data obj, uint32_t value);
int32_t cplus_data_set_as_uint64(cplus_data obj, uint64_t value);
int32_t cplus_data_set_as_float(cplus_data obj, float value);
int32_t cplus_data_set_as_double(cplus_data obj, double value);
/* int32_t cplus_data_set_as_pointer(cplus_data obj, void * value); */
int32_t cplus_data_set_as_byte_array(cplus_data obj, uint32_t array_len, uint8_t * array_bufs);
int32_t cplus_data_set_as_string(cplus_data obj, uint32_t str_len, char * str_bufs);

#ifdef __cplusplus
}
#endif
#endif // __CPLUS_DATA_H__