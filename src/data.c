/******************************************************************
* @file: data.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include "common.h"
#include "cplus.h"
#include "cplus_memmgr.h"
#include "cplus_data.h"
#include "cplus_systime.h"
#include "cplus_atomic.h"

#define OBJ_TYPE (OBJ_NONE + DS + 0)

static uint8_t spin_up = 1;
static uint8_t spin_down = 0;

#define DATA_SPIN_LOCK() \
    do { while ((spin_up == cplus_atomic_read(&(dt->spinlock))) \
        || !cplus_atomic_compare_exchange(&(dt->spinlock), \
                &(spin_down), &(spin_up))) { } \
    } while (0)

#define DATA_SPIN_UNLOCK() \
    do { cplus_atomic_write(&(dt->spinlock), spin_down); } while(0)

#define GET_VALUE_FROM_DATA_BUFS(VALUE, DATA_BUFS) \
    cplus_mem_cpy(VALUE, &(DATA_BUFS), sizeof(DATA_BUFS))

#define GET_VALUE_FROM_DATA_BUFS_SPIN(VALUE, DATA_BUFS) \
    do { DATA_SPIN_LOCK(); \
        cplus_mem_cpy(VALUE, &(DATA_BUFS), sizeof(DATA_BUFS)); \
    DATA_SPIN_UNLOCK(); } while (0)

#define GET_VALUE_FROM_DATA_BUFS_S(VALUE, DATA_BUFS) \
    cplus_atomic_write((typeof(DATA_BUFS) *)(VALUE), DATA_BUFS)

#define SET_VALUE_TO_DATA_BUFS(VALUE, DATA_BUFS) \
    cplus_mem_cpy(&DATA_BUFS, VALUE, sizeof(DATA_BUFS))

#define SET_VALUE_TO_DATA_BUFS_SPIN(VALUE, DATA_BUFS) \
     do { DATA_SPIN_LOCK(); \
        cplus_mem_cpy(&DATA_BUFS, VALUE, sizeof(DATA_BUFS)); \
    DATA_SPIN_UNLOCK(); } while (0)

#define SET_POINTER_VALUE_TO_DATA_BUFS_S(VALUE, DATA_BUFS) \
    cplus_atomic_write(&DATA_BUFS, *((typeof(DATA_BUFS) *)(VALUE)))

#define SET_VALUE_TO_DATA_BUFS_S(VALUE, DATA_BUFS) \
    cplus_atomic_write(&DATA_BUFS, ((typeof(DATA_BUFS))(VALUE)));

#define SET_VALUE_TO_POINTER_DATA_BUFS_S(VALUE, DATA_BUFS) \
    cplus_atomic_write(&DATA_BUFS, ((void *)((intptr_t)(VALUE))));

#define READ_DATA_BUFS_S(DATA_BUFS) \
    cplus_atomic_read((typeof(DATA_BUFS) *)(&(DATA_BUFS)))

#define COMPARE_DATA_BUFS_S(COMPARATOR, DATA_BUFS) \
    cplus_atomic_or((&(DATA_BUFS)), (typeof(DATA_BUFS))(COMPARATOR))

struct data
{
    uint16_t type;
    CPLUS_DATA_TYPE data_type;
    uint8_t is_valid;
    uint32_t action_mode;
    uint32_t key_len;
    char * key;
    uint8_t spinlock;
    uint32_t str_code;
    uint32_t bufs_size;
    CPLUS_DATA_VALUE value;
};

int32_t cplus_data_delete(cplus_data obj)
{
    struct data * dt = (struct data *)obj;
    CHECK_OBJECT_TYPE(obj);

    if (dt->key)
    {
        cplus_free(dt->key);
    }

    if (CPLUS_DATA_TYPE_STRING == dt->data_type)
    {
        if (dt->value.str.bufs)
        {
            cplus_free(dt->value.str.bufs);
        }
    }

    if (CPLUS_DATA_TYPE_BYTE_ARRAY == dt->data_type)
    {
        if (dt->value.byte_array.bufs)
        {
            cplus_free(dt->value.byte_array.bufs);
        }
    }

    cplus_free(dt);
    return CPLUS_SUCCESS;
}

static const char * cplus_type_string[] = {
    "NULL", "BOOL", "INT8", "INT16", "INT32", "INT64", "UINT8", "UINT16", "UINT32", "UINT64"
    , "FLOAT", "DOUBLE", "POINTER", "STRING", "BYTE_ARRAY", "UNKNOWN" };

static void * data_initialize_object(
    int type
    , void * value1
    , void * value2
    , uint32_t key_len
    , const char * key)
{
    struct data * dt = CPLUS_NULL;

    if ((dt = (struct data *)cplus_malloc(sizeof(struct data))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(dt);
        dt->type = OBJ_TYPE;
        dt->data_type = (enum cplus_data_type)(type);
        dt->is_valid = false;
        dt->key_len = 0;
        dt->key = CPLUS_NULL;
        dt->spinlock = 0;
        dt->str_code = 0;
        dt->bufs_size = 0;
        if (value1)
        {
            if (CPLUS_SUCCESS != cplus_data_set_value(dt, value1, value2))
            {
                goto exit;
            }
        }
        if (key && 0 < key_len)
        {
            if (CPLUS_SUCCESS != cplus_data_set_key(
                dt, key_len, key))
            {
                goto exit;
            }
        }
    }
    return dt;
exit:
    cplus_data_delete(dt);
    return CPLUS_NULL;
}

cplus_data cplus_data_new(CPLUS_DATA_TYPE type, void * value1, void * value2)
{
    return data_initialize_object(type, value1, value2, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_bool(bool value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_BOOL, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_int8(int8_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT8, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_int16(int16_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT16, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_int32(int32_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT32, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_int64(int64_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT64, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_uint8(uint8_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT8, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_uint16(uint16_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT16, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_uint32(uint32_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT32, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_uint64(uint64_t value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT64, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_float(float value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_FLOAT, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_double(double value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_DOUBLE, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_pointer(void * value)
{
    return data_initialize_object(CPLUS_DATA_TYPE_POINTER, &(value), CPLUS_NULL, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_string(uint32_t str_len, char * string)
{
    return data_initialize_object(CPLUS_DATA_TYPE_STRING, &(str_len), string, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_byte_array(uint32_t array_len, uint8_t * array_bufs)
{
    return data_initialize_object(CPLUS_DATA_TYPE_BYTE_ARRAY, &(array_len), array_bufs, 0, CPLUS_NULL);
}

cplus_data cplus_data_new_ex(CPLUS_DATA_TYPE type, void * value1, void * value2, uint32_t key_len, const char * key)
{
    return data_initialize_object(type, value1, value2, key_len, key);
}

cplus_data cplus_data_new_bool_ex(bool value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_BOOL, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_int8_ex(int8_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT8, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_int16_ex(int16_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT16, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_int32_ex(int32_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT32, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_int64_ex(int64_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_INT64, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_uint8_ex(uint8_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT8, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_uint16_ex(uint16_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT16, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_uint32_ex(uint32_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT32, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_uint64_ex(uint64_t value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_UINT64, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_float_ex(float value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_FLOAT, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_double_ex(double value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_DOUBLE, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_pointer_ex(void * value, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_POINTER, &(value), CPLUS_NULL, key_len, key);
}

cplus_data cplus_data_new_string_ex(uint32_t str_len, char * string, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_STRING, &(str_len), string, key_len, key);
}

cplus_data cplus_data_new_byte_array_ex(uint32_t array_len, uint8_t * array_bufs, uint32_t key_len, const char * key)
{
    return data_initialize_object(CPLUS_DATA_TYPE_BYTE_ARRAY, &(array_len), array_bufs, key_len, key);
}

bool cplus_data_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

uint32_t cplus_data_get_data_size(cplus_data obj)
{
    struct data * dt = (struct data *)(obj);
    uint32_t size = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
        {
            errno = EINVAL;
        }
        return 0;
    case CPLUS_DATA_TYPE_NULL:
        return 0;
    case CPLUS_DATA_TYPE_BOOL:
        return sizeof(bool);
    case CPLUS_DATA_TYPE_INT8:
        return sizeof(int8_t);
    case CPLUS_DATA_TYPE_INT16:
        return sizeof(int16_t);
    case CPLUS_DATA_TYPE_INT32:
        return sizeof(int32_t);
    case CPLUS_DATA_TYPE_INT64:
        return sizeof(int64_t);
    case CPLUS_DATA_TYPE_UINT8:
        return sizeof(uint8_t);
    case CPLUS_DATA_TYPE_UINT16:
        return sizeof(uint16_t);
    case CPLUS_DATA_TYPE_UINT32:
        return sizeof(uint32_t);
    case CPLUS_DATA_TYPE_UINT64:
        return sizeof(uint64_t);
    case CPLUS_DATA_TYPE_FLOAT:
        return sizeof(float);
    case CPLUS_DATA_TYPE_DOUBLE:
        return sizeof(double);
    case CPLUS_DATA_TYPE_POINTER:
        return sizeof(intptr_t);
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            size = strlen(dt->value.str.bufs);
            DATA_SPIN_UNLOCK();
        }
        return size;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            size = dt->value.byte_array.len;
            DATA_SPIN_UNLOCK();
        }
        return size;
    }
}

int32_t cplus_data_get_type(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    return READ_DATA_BUFS_S(((struct data *)(obj))->data_type);
}

char * cplus_data_get_type_str(CPLUS_DATA_TYPE type)
{
    CHECK_IN_INTERVAL(type, CPLUS_DATA_TYPE_NULL, CPLUS_DATA_TYPE_UNKNOWN, CPLUS_NULL);
    return (char *)(cplus_type_string[type]);
}

int32_t cplus_data_set_key(
    cplus_data obj
    , uint32_t key_len
    , const char * key)
{
    struct data * dt = (struct data *)(obj);

    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(key, CPLUS_FAIL);
    CHECK_GT_ZERO(key_len, CPLUS_FAIL);

    DATA_SPIN_LOCK();
    key_len += 1;
    if (key_len <= dt->key_len)
    {
        cplus_mem_cpy_ex(dt->key, dt->key_len, (void *)(key), key_len);
    }
    else
    {
        dt->key_len = key_len;
        if (CPLUS_NULL == dt->key)
        {
            if (CPLUS_NULL == (dt->key = (char *)cplus_malloc(dt->key_len)))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
        }
        else
        {
            if (CPLUS_NULL == (dt->key = (char *)cplus_realloc(dt->key, dt->key_len)))
            {
                errno = ENOBUFS;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
        }
        cplus_mem_cpy_ex(dt->key, dt->key_len, (void *)(key), dt->key_len);
    }
    DATA_SPIN_UNLOCK();

    return CPLUS_SUCCESS;
}

const char * cplus_data_get_key(cplus_data obj)
{
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    return dt->key;
}

int32_t cplus_data_set_action_mode(cplus_data obj, uint32_t mode)
{
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);

    SET_VALUE_TO_DATA_BUFS_S(mode, dt->action_mode);
    return CPLUS_SUCCESS;
}

uint32_t cplus_data_get_action_mode(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    return READ_DATA_BUFS_S(((struct data *)(obj))->action_mode);
}

cplus_data cplus_data_create_group_node(char * data_group_name)
{
    cplus_data data_group_node = CPLUS_NULL;
    cplus_llist data_group = CPLUS_NULL;

    if (CPLUS_NULL != (data_group = cplus_llist_new()))
    {
        data_group_node = cplus_data_new_pointer_ex(
            data_group, strlen(data_group_name), data_group_name);
    }
    return data_group_node;
}

cplus_llist cplus_data_get_group(cplus_data data_group_node)
{
    cplus_llist data_group = CPLUS_NULL;

    if (data_group_node && cplus_data_check(data_group_node))
    {
        data_group = cplus_data_get_pointer(data_group_node);
        if (!cplus_llist_check(data_group))
        {
            return CPLUS_NULL;
        }
    }
    return data_group;
}

int32_t cplus_data_delete_group_node(cplus_data data_group_node)
{
    cplus_llist data_group = CPLUS_NULL;

    if (data_group_node && cplus_data_check(data_group_node))
    {
        data_group = cplus_data_get_group(data_group_node);
        if (data_group && cplus_llist_check(data_group))
        {
            cplus_llist_delete(data_group);
        }
        cplus_data_delete(data_group_node);
        return CPLUS_SUCCESS;
    }
    return CPLUS_FAIL;
}

int32_t cplus_data_get_value(cplus_data obj, void * value1, void * value2)
{
    int res = CPLUS_SUCCESS;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(value1, CPLUS_FAIL);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.b);
            res = sizeof(bool);
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.i8);
            res = sizeof(int8_t);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.i16);
            res = sizeof(int16_t);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.i32);
            res = sizeof(int32_t);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            GET_VALUE_FROM_DATA_BUFS_SPIN(value1, dt->value.i64);
            res = sizeof(int64_t);
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.u8);
            res = sizeof(uint8_t);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.u16);
            res = sizeof(uint16_t);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.u32);
            res = sizeof(uint32_t);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            GET_VALUE_FROM_DATA_BUFS_SPIN(value1, dt->value.u64);
            res = sizeof(uint64_t);
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            GET_VALUE_FROM_DATA_BUFS_SPIN(value1, dt->value.f);
            res = sizeof(float);
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            GET_VALUE_FROM_DATA_BUFS_SPIN(value1, dt->value.db);
            res = sizeof(double);
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value1, dt->value.p);
            res = sizeof(void *);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            CHECK_NOT_NULL(value2, CPLUS_FAIL);
            DATA_SPIN_LOCK();
            if (* ((uint32_t *)(value1)) < (strlen(dt->value.str.bufs) + 1))
            {
                errno = ENOBUFS;
                res =  CPLUS_FAIL;
            }
            else
            {
                res = cplus_str_printf(
                    value2
                    , (* ((uint32_t *)(value1)))
                    , "%s"
                    , dt->value.str.bufs);
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            CHECK_NOT_NULL(value2, CPLUS_FAIL);
            DATA_SPIN_LOCK();
            if ((* ((uint32_t *)(value1))) < dt->value.byte_array.len)
            {
                errno = ENOBUFS;
                res =  CPLUS_FAIL;
            }
            else
            {
                if (CPLUS_NULL == cplus_mem_cpy_ex(value2
                    , (* ((uint32_t *)(value1)))
                    , dt->value.byte_array.bufs
                    , dt->value.byte_array.len))
                {
                    return CPLUS_FAIL;
                }
                res = dt->value.byte_array.len;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return res;
}

int32_t cplus_data_set_value(cplus_data obj, void * value1, void * value2)
{
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(value1, CPLUS_FAIL);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.b);
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            SET_VALUE_TO_DATA_BUFS_SPIN(value1, dt->value.i64);
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            SET_VALUE_TO_DATA_BUFS_SPIN(value1, dt->value.u64);
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            SET_VALUE_TO_DATA_BUFS_SPIN(value1, dt->value.f);
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            SET_VALUE_TO_DATA_BUFS_SPIN(value1, dt->value.db);
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_POINTER_VALUE_TO_DATA_BUFS_S(value1, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = (* ((uint32_t *)(value1))) + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((* ((uint32_t *)(value1)) + 1) > dt->value.str.len)
                {
                    dt->value.str.len = (* ((uint32_t *)(value1))) + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            else
            {
                if (value2)
                {
                    cplus_str_printf(dt->value.str.bufs
                        , dt->value.str.len, "%s", (char *)(value2));
                }
                else
                {
                    cplus_mem_set(dt->value.str.bufs, 0x00, dt->value.str.len);
                }
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            dt->bufs_size = dt->value.byte_array.len;
            dt->value.byte_array.len = (* ((uint32_t *)(value1)));
            if (CPLUS_NULL == dt->value.byte_array.bufs)
            {
                dt->bufs_size = dt->value.byte_array.len;
                dt->value.byte_array.bufs = (uint8_t *)cplus_malloc(dt->bufs_size * sizeof(uint8_t));
            }
            else
            {
                if ((* ((uint32_t *)(value1))) > dt->bufs_size)
                {
                    dt->bufs_size = dt->value.byte_array.len;
                    dt->value.byte_array.bufs = (uint8_t *)cplus_realloc(dt->value.byte_array.bufs, dt->bufs_size * sizeof(uint8_t));
                }
            }
            if (CPLUS_NULL == dt->value.byte_array.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            else
            {
                if (value2)
                {
                    cplus_mem_cpy_ex(dt->value.byte_array.bufs
                        , dt->bufs_size, value2, (* ((uint32_t *)(value1))));
                }
                else
                {
                    cplus_mem_set(dt->value.byte_array.bufs, 0x00, dt->bufs_size);
                }
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_clone_value(cplus_data dest, cplus_data src)
{
    struct data * dt_dest = (struct data *)(dest);
    struct data * dt_src = (struct data *)(src);
    CHECK_OBJECT_TYPE(dest);
    CHECK_OBJECT_TYPE(src);

    if (dt_dest->data_type != dt_src->data_type)
    {
        return CPLUS_FAIL;
    }
    else
    {
        if (CPLUS_DATA_TYPE_STRING == dt_dest->data_type)
        {
            dt_dest->str_code = dt_src->str_code;
            if (dt_src->value.str.len > dt_dest->value.str.len)
            {
                dt_dest->value.str.bufs = (char *)cplus_realloc(
                    dt_dest->value.str.bufs, dt_src->value.str.len);
                if (!(dt_dest->value.str.bufs))
                {
                    return CPLUS_FAIL;
                }
                dt_dest->value.str.len = dt_src->value.str.len;
            }
            cplus_mem_cpy(dt_dest->value.str.bufs
                , dt_src->value.str.bufs, dt_dest->value.str.len);
        }
        else if (CPLUS_DATA_TYPE_BYTE_ARRAY == dt_dest->data_type)
        {
            if (dt_src->bufs_size > dt_dest->bufs_size)
            {
                dt_dest->value.byte_array.bufs = (uint8_t *)cplus_realloc(
                    dt_dest->value.byte_array.bufs, dt_src->bufs_size);
                if (!(dt_dest->value.byte_array.bufs))
                {
                    return CPLUS_FAIL;
                }
                dt_dest->bufs_size = dt_src->bufs_size;
                dt_dest->value.byte_array.len = dt_src->value.byte_array.len;
            }
            cplus_mem_cpy(dt_dest->value.byte_array.bufs
                , dt_src->value.byte_array.bufs, dt_dest->value.byte_array.len);
        }
        else
        {
            cplus_mem_cpy(&(dt_dest->value), &(dt_src->value), sizeof(dt_dest->value));
        }
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_bool(cplus_data obj, bool value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_int8(cplus_data obj, int8_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_int16(cplus_data obj, int16_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_int32(cplus_data obj, int32_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_int64(cplus_data obj, int64_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_uint8(cplus_data obj, uint8_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_uint16(cplus_data obj, uint16_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_uint32(cplus_data obj, uint32_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_uint64(cplus_data obj, uint64_t value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_float(cplus_data obj, float value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_double(cplus_data obj, double value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_pointer(cplus_data obj, void * value)
{
    return cplus_data_set_value(obj, &(value), CPLUS_NULL);
}

int32_t cplus_data_set_string(cplus_data obj, uint32_t str_len, char * str_bufs)
{
    return cplus_data_set_value(obj, &(str_len), str_bufs);
}

int32_t cplus_data_set_byte_array(cplus_data obj, uint32_t array_len, uint8_t * array_bufs)
{
    return cplus_data_set_value(obj, &(array_len), array_bufs);
}

int32_t cplus_data_set_string_code(cplus_data obj, uint32_t code)
{
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_STRING:
        {
            SET_VALUE_TO_DATA_BUFS_S(code, dt->str_code);
        }
        break;
    }
    return CPLUS_SUCCESS;
}

bool cplus_data_get_bool(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_BOOL == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.b);
}

int8_t cplus_data_get_int8(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_INT8 == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.i8);
}

int16_t cplus_data_get_int16(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_INT16 == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.i16);
}

int32_t cplus_data_get_int32(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_INT32 == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.i32);
}

int64_t cplus_data_get_int64(cplus_data obj)
{
    int64_t i64 = 0;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_INT64 == dt->data_type);
    GET_VALUE_FROM_DATA_BUFS_SPIN(&(i64), dt->value.i64);
    return i64;
}

uint8_t cplus_data_get_uint8(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_UINT8 == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.u8);
}

uint16_t cplus_data_get_uint16(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_UINT16 == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.u16);
}

uint32_t cplus_data_get_uint32(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_UINT32 == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.u32);
}

uint64_t cplus_data_get_uint64(cplus_data obj)
{
    uint64_t u64 = 0;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_UINT64 == dt->data_type);
    GET_VALUE_FROM_DATA_BUFS_SPIN(&(u64), dt->value.u64);
    return u64;
}

float cplus_data_get_float(cplus_data obj)
{
    float f = 0;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_FLOAT == dt->data_type);
    GET_VALUE_FROM_DATA_BUFS_SPIN(&(f), dt->value.f);
    return f;
}

double cplus_data_get_double(cplus_data obj)
{
    double db = 0;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_DOUBLE == dt->data_type);
    GET_VALUE_FROM_DATA_BUFS_SPIN(&(db), dt->value.db);
    return db;
}

void * cplus_data_get_pointer(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_POINTER == ((struct data *)(obj))->data_type);
    return READ_DATA_BUFS_S(((struct data *)(obj))->value.p);
}

char * cplus_data_get_string(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_STRING == ((struct data *)(obj))->data_type);
    return ((struct data *)(obj))->value.str.bufs;
}

uint8_t * cplus_data_get_byte_array(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_BYTE_ARRAY == ((struct data *)(obj))->data_type);
    return ((struct data *)(obj))->value.byte_array.bufs;
}

uint32_t cplus_data_get_string_code(cplus_data obj)
{
    CHECK_OBJECT_TYPE(obj);
    assert(CPLUS_DATA_TYPE_STRING == ((struct data *)(obj))->data_type);

    return READ_DATA_BUFS_S(((struct data *)(obj))->str_code);
}

int32_t cplus_data_get_as_bool(cplus_data obj, bool * value)
{
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, dt->value.b);
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (0 != dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (0 != dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (0 != dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (0 != dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            (* value) = COMPARE_DATA_BUFS_S(0, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (0 >= dt->value.str.len OR CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            if (0 == strcasecmp("true", dt->value.str.bufs))
            {
                (* value) = true;
            }
            else if (0 == strcasecmp("false", dt->value.str.bufs))
            {
                (* value) = false;
            }
            else
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            if (0 >= dt->value.byte_array.len OR CPLUS_NULL == dt->value.byte_array.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            if (((uint8_t)0) < dt->value.byte_array.bufs[0])
            {
                (* value) = true;
            }
            else if (((uint8_t)0) == dt->value.byte_array.bufs[0])
            {
                (* value) = false;
            }
            else
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_int8(cplus_data obj, int8_t * value)
{
    struct data * dt = (struct data *)(obj);
    int32_t i32 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int8_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int8_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (int8_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (int8_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int8_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%d", &(i32)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = (int8_t)(i32);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_int16(cplus_data obj, int16_t * value)
{
    struct data * dt = (struct data *)(obj);
    int32_t i32 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int16_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int16_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (int16_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (int16_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int16_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%d", &(i32)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = (int16_t)(i32);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_int32(cplus_data obj, int32_t * value)
{
    struct data * dt = (struct data *)(obj);
    int32_t i32 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int32_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int32_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (int32_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (int32_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int32_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%d", &(i32)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = i32;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_int64(cplus_data obj, int64_t * value)
{
    struct data * dt = (struct data *)(obj);
    int64_t i64 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = dt->value.i64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (int64_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (int64_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (int64_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (int64_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, REG_LD, &(i64)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = i64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_uint8(cplus_data obj, uint8_t * value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t u32 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint8_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint8_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint8_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint8_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint8_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%u", &(u32)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = (uint8_t)(u32);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_uint16(cplus_data obj, uint16_t * value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t u32 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint16_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint16_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint16_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint16_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint16_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%u", &(u32)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = (uint16_t)(u32);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_uint32(cplus_data obj, uint32_t * value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t u32 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint32_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint32_t)(dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint32_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint32_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint32_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%u", &(u32)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = u32;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_uint64(cplus_data obj, uint64_t * value)
{
    struct data * dt = (struct data *)(obj);
    uint64_t u64 = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint64_t)(dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint64_t)(dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (uint64_t)(dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            GET_VALUE_FROM_DATA_BUFS_S(value, (uint64_t)((intptr_t)(dt->value.p)));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, REG_LU, &(u64)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = u64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_float(cplus_data obj, float * value)
{
    struct data * dt = (struct data *)(obj);
    float f = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.b;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.i8;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.i16;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.i32;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.i64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.u8;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.u16;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.u32;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.u64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = dt->value.f;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)dt->value.db;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            (* value) = (float)((intptr_t)(dt->value.p));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%f", &(f)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = f;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

int32_t cplus_data_get_as_double(cplus_data obj, double * value)
{
    struct data * dt = (struct data *)(obj);
    double db = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.b;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.i8;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.i16;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.i32;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.i64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.u8;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.u16;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.u32;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.u64;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)dt->value.f;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            (* value) = dt->value.db;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            (* value) = (double)((intptr_t)(dt->value.p));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(dt->value.str.bufs, "%lf", &(db)))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = db;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            cplus_mem_cpy(value
                , dt->value.byte_array.bufs
                , (dt->value.byte_array.len > sizeof((* value)))? sizeof((* value)): dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return CPLUS_SUCCESS;
}

/*
int32_t cplus_data_get_as_pointer(cplus_data obj, void ** value)
{
    struct data * dt = (struct data *)(obj);
    void *p = CPLUS_NULL;
    CHECK_OBJECT_TYPE(obj);

    DATA_SPIN_LOCK();
    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
            DATA_SPIN_UNLOCK();
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            (* value) = (void *)((intptr_t)(dt->value.b));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            (* value) = (void *)((intptr_t)(dt->value.i8));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            (* value) = (void *)((intptr_t)(dt->value.i16));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            (* value) = (void *)((intptr_t)(dt->value.i32));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            (* value) = (void *)((intptr_t)(dt->value.i64));
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            (* value) = (void *)((intptr_t)(dt->value.u8));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            (* value) = (void *)((intptr_t)(dt->value.u16));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            (* value) = (void *)((intptr_t)(dt->value.u32));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            (* value) = (void *)((intptr_t)(dt->value.u64));
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            (* value) = (void *)((intptr_t)(dt->value.f));
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            (* value) = (void *)((intptr_t)(dt->value.db));
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            (* value) = (void *)((intptr_t)(dt->value.p));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            if (1 != sscanf(dt->value.str.bufs, "%p", &p))
            {
                errno = ENOENT;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            (* value) = (void *)((intptr_t)(p));
        }
        break;
    }
    DATA_SPIN_UNLOCK();
    return CPLUS_SUCCESS;
}
*/

int32_t cplus_data_get_as_string(
    cplus_data obj
    , uint32_t str_len
    , char * str_bufs)
{
    struct data * dt = (struct data *)(obj);
    char tmp_bufs[256] = {0};
    uint32_t tmp_len = 0;

    CHECK_OBJECT_TYPE(obj);
    CHECK_GT_ZERO(str_len, CPLUS_FAIL);
    CHECK_IF_NOT(str_bufs, CPLUS_FAIL);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%s", dt->value.b? "true": "false");
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%d", dt->value.i8);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%d", dt->value.i16);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%d", dt->value.i32);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), REG_LD, dt->value.i64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%u", dt->value.u8);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%u", dt->value.u16);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%u", dt->value.u32);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), REG_LU, dt->value.u64);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%f", dt->value.f);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%lf", dt->value.db);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%p", dt->value.p);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            tmp_len = cplus_str_printf(tmp_bufs, sizeof(tmp_bufs), "%s", dt->value.str.bufs);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            if (str_len < (dt->value.byte_array.len + 1))
            {
                errno = ENOBUFS;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            else
            {
                cplus_mem_cpy_ex(str_bufs
                    , str_len
                    , dt->value.byte_array.bufs
                    , dt->value.byte_array.len);
                str_bufs[(dt->value.byte_array.len - 1)] = '\0';
                DATA_SPIN_UNLOCK();
                return dt->value.byte_array.len;
            }
        }
        break;
    }
    if(str_len < (uint32_t)(tmp_len + 1))
    {
        errno = ENOBUFS;
        return CPLUS_FAIL;
    }
    return cplus_str_printf(str_bufs, str_len, "%s", tmp_bufs);
}

int32_t cplus_data_get_as_byte_array(cplus_data obj, uint32_t array_len, uint8_t * array_bufs)
{
    struct data * dt = (struct data *)(obj);
    uint32_t len = 0;

    CHECK_OBJECT_TYPE(obj);
    CHECK_GT_ZERO(array_len, CPLUS_FAIL);
    CHECK_IF_NOT(array_bufs, CPLUS_FAIL);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(bool) > array_len)? array_len: sizeof(bool);
            cplus_mem_cpy(array_bufs, &(dt->value.b), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(int8_t) > array_len)? array_len: sizeof(int8_t);
            cplus_mem_cpy(array_bufs, &(dt->value.i8), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(int16_t) > array_len)? array_len: sizeof(int16_t);
            cplus_mem_cpy(array_bufs, &(dt->value.i16), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(int32_t) > array_len)? array_len: sizeof(int32_t);
            cplus_mem_cpy(array_bufs, &(dt->value.i32), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(int64_t) > array_len)? array_len: sizeof(int64_t);
            cplus_mem_cpy(array_bufs, &(dt->value.i64), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(uint8_t) > array_len)? array_len: sizeof(uint8_t);
            cplus_mem_cpy(array_bufs, &(dt->value.u8), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(uint16_t) > array_len)? array_len: sizeof(uint16_t);
            cplus_mem_cpy(array_bufs, &(dt->value.u16), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(uint32_t) > array_len)? array_len: sizeof(uint32_t);
            cplus_mem_cpy(array_bufs, &(dt->value.u32), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(uint64_t) > array_len)? array_len: sizeof(uint64_t);
            cplus_mem_cpy(array_bufs, &(dt->value.u64), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(float) > array_len)? array_len: sizeof(float);
            cplus_mem_cpy(array_bufs, &(dt->value.f), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(double) > array_len)? array_len: sizeof(double);
            cplus_mem_cpy(array_bufs, &(dt->value.db), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            len = (sizeof(intptr_t) > array_len)? array_len: sizeof(intptr_t);
            cplus_mem_cpy(array_bufs, &(dt->value.p), len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            len = (dt->value.str.len > array_len)? array_len: dt->value.str.len;
            cplus_mem_cpy(array_bufs
                , dt->value.str.bufs
                , len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            len = (dt->value.byte_array.len > array_len)? array_len: dt->value.byte_array.len;
            cplus_mem_cpy(array_bufs
                , dt->value.byte_array.bufs
                , len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    return len;
}

int32_t cplus_data_set_as_bool(cplus_data obj, bool value)
{
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.b);
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            cplus_data_set_string(dt
                , (value? strlen("true"): strlen("false"))
                , (value? (char *)("true"): (char *)("false")));
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(bool), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_int8(cplus_data obj, int8_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%d", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%d", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(int8_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_int16(cplus_data obj, int16_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%d", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%d", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(int16_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_int32(cplus_data obj, int32_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = (float)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%d", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%d", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(int32_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_int64(cplus_data obj, int64_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = (float)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, REG_LD, value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, REG_LD, value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(int64_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_uint8(cplus_data obj, uint8_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = (float)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%u", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%u", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(uint8_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_uint16(cplus_data obj, uint16_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = (float)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%u", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%u", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(uint16_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_uint32(cplus_data obj, uint32_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = (value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = (float)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%u", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%u", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(uint32_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_uint64(cplus_data obj, uint64_t value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i8);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i16);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.i32);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u8);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u16);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            SET_VALUE_TO_DATA_BUFS_S(value, dt->value.u32);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = (float)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            SET_VALUE_TO_POINTER_DATA_BUFS_S(value, dt->value.p);
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, REG_LU, value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, REG_LU, value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(uint64_t), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_float(cplus_data obj, float value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            dt->value.b = (0 != value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            dt->value.i8 = (int8_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            dt->value.i16 = (int16_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            dt->value.i32 = (int32_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            dt->value.i64 = (int64_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            dt->value.u8 = (uint8_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            dt->value.u16 = (uint16_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            dt->value.u32 = (uint32_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            dt->value.u64 = (uint64_t)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            dt->value.f = value;
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            dt->value.db = (double)(value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            dt->value.p = (void *)((intptr_t)(value));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            written = cplus_str_printf(CPLUS_NULL, 0, "%f", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%f", value);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(float), (uint8_t *)(&(value)));
        }
        break;
    }
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_double(cplus_data obj, double value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    DATA_SPIN_LOCK();
    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
            DATA_SPIN_UNLOCK();
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            dt->value.b = (0 != value);
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            dt->value.i8 = (int8_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            dt->value.i16 = (int16_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            dt->value.i32 = (int32_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            dt->value.i64 = (int64_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            dt->value.u8 = (uint8_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            dt->value.u16 = (uint16_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            dt->value.u32 = (uint32_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            dt->value.u64 = (uint64_t)(value);
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            dt->value.f = (float)(value);
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            dt->value.db = value;
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            dt->value.p = (void *)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            written = cplus_str_printf(CPLUS_NULL, 0, "%lf", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%lf", value);
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(double), (uint8_t *)(&(value)));
        }
        break;
    }
    DATA_SPIN_UNLOCK();
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_pointer(cplus_data obj, void * value)
{
    struct data * dt = (struct data *)(obj);
    uint32_t written = 0;
    CHECK_OBJECT_TYPE(obj);

    DATA_SPIN_LOCK();
    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
            DATA_SPIN_UNLOCK();
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            dt->value.b = (0 != ((intptr_t)(value)));
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            dt->value.i8 = (int8_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            dt->value.i16 = (int16_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            dt->value.i32 = (int32_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            dt->value.i64 = (int64_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            dt->value.u8 = (uint8_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            dt->value.u16 = (uint16_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            dt->value.u32 = (uint32_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            dt->value.u64 = (uint64_t)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            dt->value.f = (float)((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            dt->value.db = ((intptr_t)(value));
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            dt->value.p = value;
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            written = cplus_str_printf(CPLUS_NULL, 0, "%p", value);
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                dt->value.str.len = written + 1;
                dt->value.str.bufs = (char *)cplus_malloc(dt->value.str.len * sizeof(char));
            }
            else
            {
                if ((written + 1) > dt->value.str.len)
                {
                    dt->value.str.len = written + 1;
                    dt->value.str.bufs = (char *)cplus_realloc(dt->value.str.bufs, dt->value.str.len);
                }
            }
            if (CPLUS_NULL == dt->value.str.bufs)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_str_printf(dt->value.str.bufs, dt->value.str.len, "%p", value);
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, sizeof(intptr_t), (uint8_t *)(&(value)));
        }
        break;
    }
    DATA_SPIN_UNLOCK();
    dt->is_valid = true;
    return CPLUS_SUCCESS;
}

int32_t cplus_data_set_as_string(
    cplus_data obj
    , uint32_t str_len
    , char * str_bufs)
{
    int32_t res = CPLUS_FAIL;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF_NOT(str_bufs, CPLUS_FAIL);

    int32_t i32 = 0;
    int64_t i64 = 0;
    uint32_t u32 = 0;
    uint64_t u64 = 0;

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            if (0 == strcasecmp("true", str_bufs))
            {
                dt->value.b = true;
            }
            else if (0 == strcasecmp("false", str_bufs))
            {
                dt->value.b = false;
            }
            else
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%d", &(i32)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.i8 = (int8_t)(i32);
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%d", &(i32)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.i16 = (int16_t)(i32);
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%d", &(i32)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.i32 = i32;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, REG_LD, &(i64)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.i64 = i64;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%u", &(u32)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.u8 = (uint8_t)(u32);
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%u", &(u32)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.u16 = (uint16_t)(u32);
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%u", &(u32)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.u32 = u32;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, REG_LU, &(u64)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            else
            {
                dt->value.u64 = u64;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%f", &(dt->value.f)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%lf", &(dt->value.db)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            if (1 != sscanf(str_bufs, "%p", &(dt->value.p)))
            {
                errno = EINVAL;
                res = CPLUS_FAIL;
            }
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            res = cplus_data_set_string(dt, str_len, str_bufs);
        }
       break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            cplus_data_set_byte_array(dt, str_len,  (uint8_t *)(str_bufs));
        }
        break;
    }
    dt->is_valid = true;
    return res;
}

int32_t cplus_data_set_as_byte_array(cplus_data obj, uint32_t array_len, uint8_t * array_bufs)
{
    int32_t res = CPLUS_FAIL;
    struct data * dt = (struct data *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IF_NOT(array_bufs, CPLUS_FAIL);

    switch (dt->data_type)
    {
    default:
    case CPLUS_DATA_TYPE_NULL:
        {
            errno = EINVAL;
        }
        return CPLUS_FAIL;
    case CPLUS_DATA_TYPE_BOOL:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(bool))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.b), array_bufs, sizeof(dt->value.b));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT8:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(int8_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.i8), array_bufs, sizeof(dt->value.i8));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT16:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(int16_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.i16), array_bufs, sizeof(dt->value.i16));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT32:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(int32_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.i32), array_bufs, sizeof(dt->value.i32));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_INT64:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(int64_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.i64), array_bufs, sizeof(dt->value.i64));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT8:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(uint8_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.u8), array_bufs, sizeof(dt->value.u8));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT16:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(uint16_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.u16), array_bufs, sizeof(dt->value.u16));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT32:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(uint32_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.u32), array_bufs, sizeof(dt->value.u32));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_UINT64:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(uint64_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.u64), array_bufs, sizeof(dt->value.u64));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_FLOAT:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(float))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.f), array_bufs, sizeof(dt->value.f));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_DOUBLE:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(double))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.db), array_bufs, sizeof(dt->value.db));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_POINTER:
        {
            DATA_SPIN_LOCK();
            if (array_len < sizeof(intptr_t))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(&(dt->value.p), array_bufs, sizeof(dt->value.p));
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_STRING:
        {
            DATA_SPIN_LOCK();
            if (array_len < (dt->value.str.len + 1))
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(dt->value.str.bufs, array_bufs, dt->value.str.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    case CPLUS_DATA_TYPE_BYTE_ARRAY:
        {
            DATA_SPIN_LOCK();
            if (array_len < dt->value.byte_array.len)
            {
                errno = ENOMEM;
                DATA_SPIN_UNLOCK();
                return CPLUS_FAIL;
            }
            cplus_mem_cpy(dt->value.byte_array.bufs, array_bufs, dt->value.byte_array.len);
            DATA_SPIN_UNLOCK();
        }
        break;
    }
    dt->is_valid = true;
    return res;
}

bool cplus_data_is_valid(cplus_data obj)
{
    struct data * dt = (struct data *)(obj);

    if (CPLUS_NULL == dt
        OR (dt->data_type >= CPLUS_DATA_TYPE_UNKNOWN OR dt->data_type <= CPLUS_DATA_TYPE_NULL)
        OR false == dt->is_valid)
    {
        return false;
    }

    return true;
}

void cplus_data_reverse16(uint16_t * value)
{
    (* value) = ((* value) << 8) | ((* value) >> 8);
}

void cplus_data_reverse32(uint32_t * value)
{
    (* value) = ((* value) << 24)
        | (((* value) << 8) & 0x00FF0000)
        | (((* value) >> 8) & 0x0000FF00)
        | ((* value) >> 24);
}

void cplus_data_reverse64(uint64_t * value)
{
    (* value) = ((* value) << 56)
        | (((* value) << 40) & 0x00FF000000000000)
        | (((* value) << 24) & 0x0000FF0000000000)
        | (((* value) << 8)  & 0x000000FF00000000)
        | (((* value) >> 8)  & 0x00000000FF000000)
        | (((* value) >> 24) & 0x0000000000FF0000)
        | (((* value) >> 40) & 0x000000000000FF00)
        | ((* value) >> 56);
}

uint16_t cplus_data_swap16(uint16_t value)
{
    return (value << 8) | (value >> 8);
}

uint32_t cplus_data_swap32(uint32_t value)
{
    return (value << 24)
        | ((value << 8) & 0x00FF0000)
        | ((value >> 8) & 0x0000FF00)
        | (value >> 24);
}

uint64_t cplus_data_swap64(uint64_t value)
{
    return (value << 56)
        | ((value << 40) & 0x00FF000000000000)
        | ((value << 24) & 0x0000FF0000000000)
        | ((value << 8)  & 0x000000FF00000000)
        | ((value >> 8)  & 0x00000000FF000000)
        | ((value >> 24) & 0x0000000000FF0000)
        | ((value >> 40) & 0x000000000000FF00)
        | (value >> 56);
}

#ifdef __CPLUS_UNITTEST__
#include <math.h>

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_BOOL)
{
    bool result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_bool(true)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool"), "test_bool"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool_bool_bool_bool"), "test_bool_bool_bool_bool"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_bool_bool_bool_bool", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(bool), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(true, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_bool(data, false));
    UNITTEST_EXPECT_EQ(false, cplus_data_get_bool(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT8)
{
    int8_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int8(0xab)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int8"), "test_int8"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT8, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xab, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int8(data, 0xba));
    UNITTEST_EXPECT_EQ(0xba, cplus_data_get_int8(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT16)
{
    int16_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int16(0xabcd)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int16"), "test_int16"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT16, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xabcd, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int16(data, 0xdcba));
    UNITTEST_EXPECT_EQ(0xdcba, cplus_data_get_int16(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT32)
{
    int32_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32(123456)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int32"), "test_int32"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT32, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(123456, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int32(data, -123456));
    UNITTEST_EXPECT_EQ(-123456, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT64)
{
    int64_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int64(0x0123abcddcba3210)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int64"), "test_int64"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT64, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int64(data, 0x7FFFFFFFFFFFFFFF));
    UNITTEST_EXPECT_EQ(0x7FFFFFFFFFFFFFFF, cplus_data_get_int64(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT8)
{
    uint8_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint8(0xab)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint8"), "test_uint8"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xab, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint8(data, 0xba));
    UNITTEST_EXPECT_EQ(0xba, cplus_data_get_uint8(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT16)
{
    uint16_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint16(0xabcd)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint16"), "test_uint16"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xabcd, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint16(data, 0xdcba));
    UNITTEST_EXPECT_EQ(0xdcba, cplus_data_get_uint16(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT32)
{
    uint32_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint32(123456)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint32"), "test_uint32"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(123456, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint32(data, 6543210));
    UNITTEST_EXPECT_EQ(6543210, cplus_data_get_uint32(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT64)
{
    uint64_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint64(0x0123abcddcba3210)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint64"), "test_uint64"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, result);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint64(data, 0xdcba32100123abcd));
    UNITTEST_EXPECT_EQ(0xdcba32100123abcd, cplus_data_get_uint64(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_FLOAT)
{
    float result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_float(3.1415926535)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_float"), "test_float"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(float), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_FLOAT, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(float), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - result) < 0.0000001f);
    UNITTEST_EXPECT_EQ(0, result - 3.14159);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_float(data, 2.7182818284));
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_float(data)) < 0.01f);
    UNITTEST_EXPECT_EQ(0, cplus_data_get_float(data) - 2.71828);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_DOUBLE)
{
    double result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_double(3.1415926535)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_double"), "test_double"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(double), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_DOUBLE, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(double), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - result) < 0.0000001f);
    UNITTEST_EXPECT_EQ(0, result - 3.14159);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_double(data, 2.7182818284));
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_double(data)) < 0.0000001f);
    UNITTEST_EXPECT_EQ(0, cplus_data_get_double(data) - 2.71828);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_POINTER)
{
    int32_t v1 = 123456, v2 = 654321;
    void * result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_pointer(&v1)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_pointer"), "test_pointer"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(intptr_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_POINTER, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(123456, *((int32_t *)result));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_pointer(data, &v2));
    UNITTEST_EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(654321, *((int32_t *)result));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_pointer(data, &v1));
    UNITTEST_EXPECT_EQ(true, (CPLUS_NULL != (result = cplus_data_get_pointer(data))))
    UNITTEST_EXPECT_EQ(123456, *((int32_t *)result));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_STRING)
{
    char bufs[32];
    uint32_t len = sizeof(bufs);

    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_string(strlen("Hello World"), (char *)("Hello World"))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_string"), (char *)("test_string")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(strlen("Hello World"), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_STRING, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(strlen("Hello World"), cplus_data_get_value(data, &len, bufs));
    UNITTEST_EXPECT_EQ(0, strcmp("Hello World", bufs));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_string(data, strlen("AaBbCcDd"), (char *)("AaBbCcDd")));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd"), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd"), strlen(cplus_data_get_string(data)));
    UNITTEST_EXPECT_EQ(0, strcmp("AaBbCcDd", cplus_data_get_string(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_string_code(data, 0xabcd));
    UNITTEST_EXPECT_EQ(0xabcd, cplus_data_get_string_code(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_BYTE_ARRAY)
{
    uint8_t bufs[32] = {0}, get_value[32] = {0}, test_value[64] = {0};
    uint32_t get_len = 32;
    cplus_data data = CPLUS_NULL;

    for (uint32_t i = 0; i < sizeof(bufs); i ++)
    {
        bufs[i] = 0xAB;
    }
    test_value[0] = 0xff;

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_byte_array(sizeof(bufs), bufs)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_byte_array"), "test_byte_array"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_byte_array", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(bufs), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_BYTE_ARRAY, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(bufs), cplus_data_get_value(data, &get_len, get_value));
    UNITTEST_EXPECT_EQ(0, memcmp(bufs, get_value, sizeof(bufs)));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_byte_array(data, 1, test_value));
    UNITTEST_EXPECT_EQ(1, cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(0, memcmp(test_value, cplus_data_get_byte_array(data), 1));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_byte_array(data, 1, test_value));
    UNITTEST_EXPECT_EQ(1, cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(0, memcmp(test_value, cplus_data_get_byte_array(data), 1));
    for (uint32_t i = 0; i < sizeof(test_value); i ++)
    {
        test_value[i] = 0xBC;
    }
    UNITTEST_EXPECT_EQ(0, cplus_data_set_byte_array(data, sizeof(test_value), test_value));
    UNITTEST_EXPECT_EQ(sizeof(test_value), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(0, memcmp(test_value, cplus_data_get_byte_array(data), sizeof(test_value)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_BOOL)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_bool(true)));
    UNITTEST_EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, 1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 1 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 1 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 1 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 1 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 1 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 1 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 1 == db);
    UNITTEST_EXPECT_EQ(strlen("true"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "true"));
    UNITTEST_EXPECT_EQ(1, cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0x01;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, 1)));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_bool(data, false));
    UNITTEST_EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, false == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, 0 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 0 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 0 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 0 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 0 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 0 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 0 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 0 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 0 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 0 == db);
    UNITTEST_EXPECT_EQ(strlen("false"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "false"));
    UNITTEST_EXPECT_EQ(1, cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0x00;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, 1)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT8)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int8(127)));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT8, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, 127 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 127 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 127 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 127 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 127 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 127 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 127 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 127 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 127 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 127 == db);
    UNITTEST_EXPECT_EQ(strlen("127"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "127"));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int8_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int8(data, -1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, -1 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, (uint64_t)(-1) == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, -1 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, -1 == db);
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "-1"));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int8_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT16)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int16(32767)));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT16, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 32767 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 32767 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 32767 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 32767 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 32767 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 32767 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 32767 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 32767 == db);
    UNITTEST_EXPECT_EQ(strlen("32767"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "32767"));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int16_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int16(data, -1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, -1 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, (uint64_t)(-1) == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, -1 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, -1 == db);
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "-1"));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int16_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT32)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32(2147483647)));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT32, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 2147483647 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 2147483647 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 2147483647 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 2147483647 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 2147483647 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 2147483647 == db);
    UNITTEST_EXPECT_EQ(strlen("2147483647"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "2147483647"));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int32_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int32(data, -1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, -1 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, (uint64_t)(-1) == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, -1 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, -1 == db);
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "-1"));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int32_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT64)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int64(0x7fffffffffffffff)));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT64, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == db);
    UNITTEST_EXPECT_EQ(strlen("9223372036854775807"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "9223372036854775807"));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0xff;
    test_value[4] = 0xff;
    test_value[5] = 0xff;
    test_value[6] = 0xff;
    test_value[7] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int64_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int64(data, -1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, -1 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, (uint64_t)(-1) == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, -1 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, -1 == db);
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "-1"));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0xff;
    test_value[4] = 0xff;
    test_value[5] = 0xff;
    test_value[6] = 0xff;
    test_value[7] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(int64_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT8)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint8(127)));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, 127 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 127 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 127 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 127 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 127 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 127 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 127 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 127 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 127 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 127 == db);
    UNITTEST_EXPECT_EQ(strlen("127"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "127"));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint8_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint8(data, 255));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 255 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 255 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 255 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 255 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 255 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 255 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 255 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 255 == db);
    UNITTEST_EXPECT_EQ(strlen("255"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "255"));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint8_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT16)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint16(32767)));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, 32767 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 32767 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 32767 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 32767 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 32767 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 32767 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 32767 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 32767 == db);
    UNITTEST_EXPECT_EQ(strlen("32767"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "32767"));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint16_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint16(data, 65535));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 65535 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 65535 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 65535 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 65535 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 65535 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 65535 == db);
    UNITTEST_EXPECT_EQ(strlen("65535"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "65535"));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint16_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT32)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint32(2147483647)));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, 2147483647 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 2147483647 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 2147483647 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 2147483647 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 2147483647 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 2147483647 == db);
    UNITTEST_EXPECT_EQ(strlen("2147483647"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "2147483647"));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint32_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint32(data, 4294967295));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 4294967295 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 4294967295 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 4294967295 == db);
    UNITTEST_EXPECT_EQ(strlen("4294967295"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "4294967295"));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint32_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT64)
{
    cplus_data data = CPLUS_NULL;
    bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint64(0x7fffffffffffffff)));
    UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 9223372036854775807 == db);
    UNITTEST_EXPECT_EQ(strlen("9223372036854775807"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "9223372036854775807"));
    UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0xff;
    test_value[4] = 0xff;
    test_value[5] = 0xff;
    test_value[6] = 0xff;
    test_value[7] = 0x7f;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint64_t))));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint64(data, 0xffffffffffffffff));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
    UNITTEST_EXPECT_EQ(true, true == b);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
    UNITTEST_EXPECT_EQ(true, -1 == i8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
    UNITTEST_EXPECT_EQ(true, -1 == i16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
    UNITTEST_EXPECT_EQ(true, -1 == i32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
    UNITTEST_EXPECT_EQ(true, -1 == i64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
    UNITTEST_EXPECT_EQ(true, 255 == u8);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
    UNITTEST_EXPECT_EQ(true, 65535 == u16);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
    UNITTEST_EXPECT_EQ(true, 4294967295 == u32);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
    UNITTEST_EXPECT_EQ(true, 0xffffffffffffffff == u64);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
    UNITTEST_EXPECT_EQ(true, 0xffffffffffffffff == f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
    UNITTEST_EXPECT_EQ(true, 0xffffffffffffffff == db);
    UNITTEST_EXPECT_EQ(strlen("18446744073709551615"), cplus_data_get_as_string(data, len, str));
    UNITTEST_EXPECT_EQ(true, 0 == strcmp(str, "18446744073709551615"));
     UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
    test_value[0] = 0xff;
    test_value[1] = 0xff;
    test_value[2] = 0xff;
    test_value[3] = 0xff;
    test_value[4] = 0xff;
    test_value[5] = 0xff;
    test_value[6] = 0xff;
    test_value[7] = 0xff;
    UNITTEST_EXPECT_EQ(true, (0 == memcmp(bufs, test_value, sizeof(uint64_t))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_set_as, CPLUS_DATA_TYPE_STRING)
{
    cplus_data data = CPLUS_NULL;
    char str[64]; uint32_t len = sizeof(str);
    /* char cmp[64]; uint32_t cmp_len = 0, u32 = 5566; */
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_string(strlen("123"), (char *)("123"))));
    UNITTEST_EXPECT_EQ(strlen("123"), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_STRING, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_bool(data, true));
    UNITTEST_EXPECT_EQ(strlen("true"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("true", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint8(data, 0xff));
    UNITTEST_EXPECT_EQ(strlen("255"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("255", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint16(data, 0xffff));
    UNITTEST_EXPECT_EQ(strlen("65535"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("65535", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint32(data, 0xffffffff));
    UNITTEST_EXPECT_EQ(strlen("4294967295"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("4294967295", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint64(data, 0xffffffffffffffff));
    UNITTEST_EXPECT_EQ(strlen("18446744073709551615"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("18446744073709551615", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_bool(data, false));
    UNITTEST_EXPECT_EQ(strlen("false"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("false", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int8(data, 0xff));
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("-1", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int16(data, 0xffff));
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("-1", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int32(data, 0xffffffff));
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("-1", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int64(data, 0xffffffffffffffff));
    UNITTEST_EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("-1", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_float(data, 3.141593));
    UNITTEST_EXPECT_EQ(strlen("3.141593"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("3.141593", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_double(data, 3.141593));
    UNITTEST_EXPECT_EQ(strlen("3.141593"), cplus_data_get_value(data, &len, str));
    /*
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_pointer(data, &u32));
    UNITTEST_EXPECT_EQ(true,  0 < (cmp_len = snprintf(cmp, sizeof(cmp), "%p", &u32)));
    UNITTEST_EXPECT_EQ(cmp_len, cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp(cmp, str));
    */
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_string(data,
    strlen("012345678901234567890123456789012345678901234567890123456789123"), (char *)("012345678901234567890123456789012345678901234567890123456789123")));
    UNITTEST_EXPECT_EQ(strlen("012345678901234567890123456789012345678901234567890123456789123"), cplus_data_get_value(data, &len, str));
    UNITTEST_EXPECT_EQ(0, strcmp("012345678901234567890123456789012345678901234567890123456789123", str));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_is_valid, functionity)
{
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new(CPLUS_DATA_TYPE_NULL, CPLUS_NULL, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_bool(true)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int8(0xab)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int16(0xabcd)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32(123456)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int64(0x0123abcddcba3210)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint8(0xab)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint16(0xabcd)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint32(123456)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint64(0x0123abcddcba3210)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_float(3.1415926535)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_double(3.1415926535)));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_string(strlen("Hello World"), (char *)("Hello World"))));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_NULL, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_BOOL, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_bool(data, true));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_INT8, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int8(data, 0xab));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_INT16, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int16(data, 0xabcd));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_INT32, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int32(data, 123456));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_INT64, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int64(data, 0x0123abcddcba3210));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_UINT8, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint8(data, 0xab));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_UINT16, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint16(data, 0xabcd));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_UINT32, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint32(data, 123456));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_UINT64, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint64(data, 0x0123abcddcba3210));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_FLOAT, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_float(data, 3.1415926535));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_DOUBLE, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_double(data, 3.1415926535));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_POINTER, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_ex(
        CPLUS_DATA_TYPE_STRING, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(false, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_string(data, strlen("Hello World"), (char *)("Hello World")));
    UNITTEST_EXPECT_EQ(true, cplus_data_is_valid(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_reverse, functionity)
{
    int16_t i16 = 0x0123;
    int32_t i32 = 0x01234567;
    int64_t i64 = 0x0123456789abcdef;
    uint16_t u16 = 0x0123;
    uint32_t u32 = 0x01234567;
    uint64_t u64 = 0x0123456789abcdef;

    UNITTEST_EXPECT_EQ(0x0123, i16);
    UNITTEST_EXPECT_EQ(0x0123, u16);
    UNITTEST_EXPECT_EQ(0x2301, CPLUS_DATA_SWAP(i16));
    UNITTEST_EXPECT_EQ(0x2301, CPLUS_DATA_SWAP(u16));
    UNITTEST_EXPECT_EQ(0x0123, i16);
    UNITTEST_EXPECT_EQ(0x0123, u16);
    UNITTEST_EXPECT_EQ(0x01234567, i32);
    UNITTEST_EXPECT_EQ(0x01234567, u32);
    UNITTEST_EXPECT_EQ(0x67452301, CPLUS_DATA_SWAP(i32));
    UNITTEST_EXPECT_EQ(0x67452301, CPLUS_DATA_SWAP(u32));
    UNITTEST_EXPECT_EQ(0x01234567, i32);
    UNITTEST_EXPECT_EQ(0x01234567, u32);
    UNITTEST_EXPECT_EQ(0x0123456789abcdef, i64);
    UNITTEST_EXPECT_EQ(0x0123456789abcdef, u64);
    UNITTEST_EXPECT_EQ(0xefcdab8967452301, CPLUS_DATA_SWAP(i64));
    UNITTEST_EXPECT_EQ(0xefcdab8967452301, CPLUS_DATA_SWAP(u64));
    UNITTEST_EXPECT_EQ(0x0123456789abcdef, i64);
    UNITTEST_EXPECT_EQ(0x0123456789abcdef, u64);
    UNITTEST_EXPECT_EQ(0x0123, i16);
    UNITTEST_EXPECT_EQ(0x0123, u16);
    CPLUS_DATA_REVERSE(&i16);
    CPLUS_DATA_REVERSE(&u16);
    UNITTEST_EXPECT_EQ(0x2301, i16);
    UNITTEST_EXPECT_EQ(0x2301, u16);
    UNITTEST_EXPECT_EQ(0x01234567, i32);
    UNITTEST_EXPECT_EQ(0x01234567, u32);
    CPLUS_DATA_REVERSE(&i32);
    CPLUS_DATA_REVERSE(&u32);
    UNITTEST_EXPECT_EQ(0x67452301, i32);
    UNITTEST_EXPECT_EQ(0x67452301, u32);
    UNITTEST_EXPECT_EQ(0x0123456789abcdef, i64);
    UNITTEST_EXPECT_EQ(0x0123456789abcdef, u64);
    CPLUS_DATA_REVERSE(&i64);
    CPLUS_DATA_REVERSE(&u64);
    UNITTEST_EXPECT_EQ(0xefcdab8967452301, i64);
    UNITTEST_EXPECT_EQ(0xefcdab8967452301, u64);
}

CPLUS_UNIT_TEST(cplus_data_new_ex, all_type)
{
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_bool_ex(true, strlen("test_bool"), "test_bool")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int8_ex(0xab, strlen("test_int8"), "test_int8")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int16_ex(0xabcd, strlen("test_int16"), "test_int16")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32_ex(123456, strlen("test_int32"), "test_int32")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int64_ex(0x0123abcddcba3210, strlen("test_int64"), "test_int64")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint8_ex(0xab, strlen("test_uint8"), "test_uint8")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint16_ex(0xabcd, strlen("test_uint16"), "test_uint16")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint32_ex(123456, strlen("test_uint32"), "test_uint32")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint64_ex(0x0123abcddcba3210, strlen("test_uint64"), "test_uint64")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_float_ex(3.1415926535, strlen("test_float"), "test_float")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_double_ex(3.1415926535, strlen("test_double"), "test_double")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    int32_t v1 = 123456;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_pointer_ex(&v1, strlen("test_pointer"), "test_pointer")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_string_ex(strlen("Hello World"), (char *)("Hello World"), strlen("test_string"), "test_string")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new_ex, thread_safe)
{
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_bool_ex(true, strlen("test_bool"), "test_bool")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int8_ex(0xab, strlen("test_int8"), "test_int8")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int16_ex(0xabcd, strlen("test_int16"), "test_int16")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32_ex(123456, strlen("test_int32"), "test_int32")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int64_ex(0x0123abcddcba3210, strlen("test_int64"), "test_int64")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint8_ex(0xab, strlen("test_uint8"), "test_uint8")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint16_ex(0xabcd, strlen("test_uint16"), "test_uint16")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint32_ex(123456, strlen("test_uint32"), "test_uint32")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint64_ex(0x0123abcddcba3210, strlen("test_uint64"), "test_uint64")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_float_ex(3.1415926535, strlen("test_float"), "test_float")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_double_ex(3.1415926535, strlen("test_double"), "test_double")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    int32_t v1 = 123456;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_pointer_ex(&v1, strlen("test_pointer"), "test_pointer")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_string_ex(strlen("Hello World"), (char *)("Hello World"), strlen("test_string"), "test_string")));
    UNITTEST_EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_BOOL_THREAD_SAFE)
{
    bool result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_bool_ex(true, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool"), "test_bool"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool_bool_bool_bool"), "test_bool_bool_bool_bool"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_bool_bool_bool_bool", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(bool), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(true, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_bool(data, false));
    UNITTEST_EXPECT_EQ(false, cplus_data_get_bool(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT8_THREAD_SAFE)
{
    int8_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int8_ex(0xab, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int8"), "test_int8"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT8, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xab, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int8(data, 0xba));
    UNITTEST_EXPECT_EQ(0xba, cplus_data_get_int8(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT16_THREAD_SAFE)
{
    int16_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int16_ex(0xabcd, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int16"), "test_int16"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT16, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xabcd, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int16(data, 0xdcba));
    UNITTEST_EXPECT_EQ(0xdcba, cplus_data_get_int16(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT32_THREAD_SAFE)
{
    int32_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32_ex(123456, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int32"), "test_int32"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT32, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(123456, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int32(data, -123456));
    UNITTEST_EXPECT_EQ(-123456, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_INT64_THREAD_SAFE)
{
    int64_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int64_ex(0x0123abcddcba3210, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int64"), "test_int64"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT64, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(int64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_int64(data, 0x7FFFFFFFFFFFFFFF));
    UNITTEST_EXPECT_EQ(0x7FFFFFFFFFFFFFFF, cplus_data_get_int64(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT8_THREAD_SAFE)
{
    uint8_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint8_ex(0xab, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint8"), "test_uint8"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xab, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint8(data, 0xba));
    UNITTEST_EXPECT_EQ(0xba, cplus_data_get_uint8(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT16_THREAD_SAFE)
{
    uint16_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint16_ex(0xabcd, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint16"), "test_uint16"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0xabcd, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint16(data, 0xdcba));
    UNITTEST_EXPECT_EQ(0xdcba, cplus_data_get_uint16(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT32_THREAD_SAFE)
{
    uint32_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint32_ex(123456, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint32"), "test_uint32"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(123456, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint32(data, 6543210));
    UNITTEST_EXPECT_EQ(6543210, cplus_data_get_uint32(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT64_THREAD_SAFE)
{
    uint64_t result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_uint64_ex(0x0123abcddcba3210, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint64"), "test_uint64"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(uint64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, result);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_uint64(data, 0xdcba32100123abcd));
    UNITTEST_EXPECT_EQ(0xdcba32100123abcd, cplus_data_get_uint64(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_FLOAT_THREAD_SAFE)
{
    float result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_float_ex(3.1415926535, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_float"), "test_float"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(float), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_FLOAT, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(float), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - result) < 0.0000001f);
    UNITTEST_EXPECT_EQ(0, result - 3.14159);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_float(data, 2.7182818284));
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_float(data)) < 0.01f);
    UNITTEST_EXPECT_EQ(0, cplus_data_get_float(data) - 2.71828);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_DOUBLE_THREAD_SAFE)
{
    double result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_double_ex(3.1415926535, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_double"), "test_double"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(double), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_DOUBLE, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(double), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - result) < 0.0000001f);
    UNITTEST_EXPECT_EQ(0, result - 3.14159);
    UNITTEST_EXPECT_EQ(0, cplus_data_set_double(data, 2.7182818284));
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_double(data)) < 0.0000001f);
    UNITTEST_EXPECT_EQ(0, cplus_data_get_double(data) - 2.71828);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_POINTER_THREAD_SAFE)
{
    int32_t v1 = 123456, v2 = 654321;
    void * result;
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_pointer_ex(&v1, 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_pointer"), "test_pointer"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(sizeof(intptr_t), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_POINTER, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(123456, *((int32_t *)result));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_pointer(data, &v2));
    UNITTEST_EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
    UNITTEST_EXPECT_EQ(654321, *((int32_t *)result));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_new, CPLUS_DATA_TYPE_STRING_THREAD_SAFE)
{
    char bufs[32];
    uint32_t len = sizeof(bufs);

    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_string_ex(strlen("Hello World"), (char *)("Hello World"), 0, CPLUS_NULL)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_string"), "test_string"));
    UNITTEST_EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
    UNITTEST_EXPECT_EQ(strlen("Hello World"), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_STRING, cplus_data_get_type(data));
    UNITTEST_EXPECT_EQ(strlen("Hello World"), cplus_data_get_value(data, &len, bufs));
    UNITTEST_EXPECT_EQ(0, strcmp("Hello World", bufs));
    UNITTEST_EXPECT_EQ(0, cplus_data_set_string(data, strlen("AaBbCcDd"), (char *)("AaBbCcDd")));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd"), cplus_data_get_data_size(data));
    UNITTEST_EXPECT_EQ(strlen("AaBbCcDd"), strlen(cplus_data_get_string(data)));
    UNITTEST_EXPECT_EQ(0, strcmp("AaBbCcDd", cplus_data_get_string(data)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_set_action_mode, functionity)
{
    cplus_data data = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data = cplus_data_new_int32(123456)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_action_mode(data, 654321));
    UNITTEST_EXPECT_EQ(654321, cplus_data_get_action_mode(data));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_create_group_node, functionity)
{
    cplus_data group_node = CPLUS_NULL;
    cplus_llist group = CPLUS_NULL;
    char test[] = "test";
    int32_t i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;

    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (group_node = cplus_data_create_group_node(test)));
    UNITTEST_EXPECT_EQ(true, (true == cplus_data_check(group_node)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (group = cplus_data_get_group(group_node)));
    UNITTEST_EXPECT_EQ(true, (true == cplus_llist_check(group)));
    UNITTEST_EXPECT_EQ(0, strcmp(test, cplus_data_get_key(group_node)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i5));
    UNITTEST_EXPECT_EQ(i1, *((int32_t *)cplus_llist_pop_front(group)));
    UNITTEST_EXPECT_EQ(i2, *((int32_t *)cplus_llist_pop_front(group)));
    UNITTEST_EXPECT_EQ(i3, *((int32_t *)cplus_llist_pop_front(group)));
    UNITTEST_EXPECT_EQ(i4, *((int32_t *)cplus_llist_pop_front(group)));
    UNITTEST_EXPECT_EQ(i5, *((int32_t *)cplus_llist_pop_front(group)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete_group_node(group_node));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_data_clone_value, functionity)
{
    cplus_data data1 = CPLUS_NULL, data2 = CPLUS_NULL;
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_bool(true)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_bool(false)));
    UNITTEST_EXPECT_EQ(true, cplus_data_get_bool(data1));
    UNITTEST_EXPECT_EQ(false, cplus_data_get_bool(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(true, true == cplus_data_get_bool(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_int8(0xAB)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_int8(0xCD)));
    UNITTEST_EXPECT_EQ(0xAB, cplus_data_get_int8(data1));
    UNITTEST_EXPECT_EQ(0xCD, cplus_data_get_int8(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0xAB, cplus_data_get_int8(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_uint8(0xAB)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_uint8(0xCD)));
    UNITTEST_EXPECT_EQ(0xAB, cplus_data_get_uint8(data1));
    UNITTEST_EXPECT_EQ(0xCD, cplus_data_get_uint8(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0xAB, cplus_data_get_uint8(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_int16(0x1234)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_int16(0x5678)));
    UNITTEST_EXPECT_EQ(0x1234, cplus_data_get_int16(data1));
    UNITTEST_EXPECT_EQ(0x5678, cplus_data_get_int16(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0x1234, cplus_data_get_int16(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_uint16(0x1234)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_uint16(0x5678)));
    UNITTEST_EXPECT_EQ(0x1234, cplus_data_get_uint16(data1));
    UNITTEST_EXPECT_EQ(0x5678, cplus_data_get_uint16(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0x1234, cplus_data_get_uint16(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_int32(0x12345678)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_int32(0x5678abcd)));
    UNITTEST_EXPECT_EQ(0x12345678, cplus_data_get_int32(data1));
    UNITTEST_EXPECT_EQ(0x5678abcd, cplus_data_get_int32(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0x12345678, cplus_data_get_int32(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_uint32(0x12345678)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_uint32(0x5678abcd)));
    UNITTEST_EXPECT_EQ(0x12345678, cplus_data_get_uint32(data1));
    UNITTEST_EXPECT_EQ(0x5678abcd, cplus_data_get_uint32(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0x12345678, cplus_data_get_uint32(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_int64(0x0123abcddcba3210)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_int64(0xabcdef0123456789)));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, cplus_data_get_int64(data1));
    UNITTEST_EXPECT_EQ(0xabcdef0123456789, cplus_data_get_int64(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, cplus_data_get_int64(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_uint64(0x0123abcddcba3210)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_uint64(0xabcdef0123456789)));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, cplus_data_get_uint64(data1));
    UNITTEST_EXPECT_EQ(0xabcdef0123456789, cplus_data_get_uint64(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(0x0123abcddcba3210, cplus_data_get_uint64(data2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_float(3.1415926535)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_float(2.7182818284)));
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - cplus_data_get_float(data1)) < 0.01f);
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_float(data2)) < 0.01f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - cplus_data_get_float(data2)) < 0.01f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_double(2.7182818284)));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_double(3.1415926535)));
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_double(data1)) < 0.0000001f);
    UNITTEST_EXPECT_EQ(true, fabs(3.1415926535 - cplus_data_get_double(data2)) < 0.0000001f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
    UNITTEST_EXPECT_EQ(true, fabs(2.7182818284 - cplus_data_get_double(data2)) < 0.0000001f);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data1 = cplus_data_new_string(strlen("Hello World"), (char *)("Hello World"))));
    UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (data2 = cplus_data_new_string(strlen("AaBbCcDd 01234567"), (char *)("AaBbCcDd 01234567"))));
    UNITTEST_EXPECT_EQ(0, strcmp("Hello World", cplus_data_get_string(data1)));
    UNITTEST_EXPECT_EQ(0, strcmp("AaBbCcDd 01234567", cplus_data_get_string(data2)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data1, data2));
    UNITTEST_EXPECT_EQ(0, strcmp("AaBbCcDd 01234567", cplus_data_get_string(data1)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

#ifndef __cplusplus
CPLUS_UNIT_TEST(CPLUS_DATA_TYPE_CONV, functionity)
{
    bool b; float f; double db; char str[64];
    int8_t i8; int16_t i16; int32_t i32; int64_t i64;
    uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
    void *p;

    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, CPLUS_DATA_TYPE_CONV(b));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT8, CPLUS_DATA_TYPE_CONV(i8));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT16, CPLUS_DATA_TYPE_CONV(i16));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT32, CPLUS_DATA_TYPE_CONV(i32));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_INT64, CPLUS_DATA_TYPE_CONV(i64));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, CPLUS_DATA_TYPE_CONV(u8));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, CPLUS_DATA_TYPE_CONV(u16));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, CPLUS_DATA_TYPE_CONV(u32));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, CPLUS_DATA_TYPE_CONV(u64));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_FLOAT, CPLUS_DATA_TYPE_CONV(f));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_DOUBLE, CPLUS_DATA_TYPE_CONV(db));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_POINTER, CPLUS_DATA_TYPE_CONV(p));
    UNITTEST_EXPECT_EQ(CPLUS_DATA_TYPE_STRING, CPLUS_DATA_TYPE_CONV(str));
}
#endif

void unittest_data(void)
{
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_BOOL);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT8);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT16);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT32);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT64);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT8);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT16);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT32);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT64);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_FLOAT);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_DOUBLE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_POINTER);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_STRING);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_BOOL);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_INT8);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_INT16);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_INT32);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_INT64);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_UINT8);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_UINT16);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_UINT32);
    UNITTEST_ADD_TESTCASE(cplus_data_get_as, CPLUS_DATA_TYPE_UINT64);
    UNITTEST_ADD_TESTCASE(cplus_data_set_as, CPLUS_DATA_TYPE_STRING);
    UNITTEST_ADD_TESTCASE(cplus_data_is_valid, functionity);
    UNITTEST_ADD_TESTCASE(cplus_data_reverse, functionity);
    UNITTEST_ADD_TESTCASE(cplus_data_new_ex, all_type);
    UNITTEST_ADD_TESTCASE(cplus_data_new_ex, thread_safe);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_BOOL_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT8_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT16_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT32_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_INT64_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT8_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT16_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT32_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_UINT64_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_FLOAT_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_DOUBLE_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_POINTER_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_STRING_THREAD_SAFE);
    UNITTEST_ADD_TESTCASE(cplus_data_set_action_mode, functionity);
    UNITTEST_ADD_TESTCASE(cplus_data_create_group_node, functionity);
    UNITTEST_ADD_TESTCASE(cplus_data_clone_value, functionity);
#ifndef __cplusplus
    UNITTEST_ADD_TESTCASE(CPLUS_DATA_TYPE_CONV, functionity);
#endif
    UNITTEST_ADD_TESTCASE(cplus_data_new, CPLUS_DATA_TYPE_BYTE_ARRAY);
}
#endif // __CPLUS_UNITTEST__
