/******************************************************************
* @file: llist.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include "common.h"
#include "cplus.h"
#include "cplus_memmgr.h"
#include "cplus_mempool.h"
#include "cplus_llist.h"
#include "cplus_rwlock.h"

#define OBJ_TYPE (OBJ_NONE + DS + 1)

#define HEAD 0
#define SIZE get_size(list)
#define TAIL (SIZE - 1)
#define MAX_NODE_COUNT (1024 * 1024)

struct llist_node
{
    void * data;
    struct llist_node * prev;
    struct llist_node * next;
};

struct linked_list
{
    uint16_t type;
    uint32_t count;
    uint32_t sort_count;
    int32_t cur_index;
    cplus_mempool mempool;
    cplus_rwlock lock;
    struct llist_node * head;
    struct llist_node * tail;
    struct llist_node ** current;
};

static void * llist_initialize_object(uint32_t max_count, bool thread_safe)
{
    struct linked_list * list = NULL;

    if ((list = (struct linked_list *)cplus_malloc(sizeof(struct linked_list))))
    {
        CPLUS_INITIALIZE_STRUCT_POINTER(list);

        list->type = OBJ_TYPE;
        list->count = 0;
        list->sort_count = 0;
        list->mempool = cplus_mempool_new(
            max_count
            , sizeof(struct llist_node));
        if (NULL == list->mempool AND 0 != max_count)
        {
            errno = ENOMEM;
            goto exit;
        }
        list->head = NULL;
        list->tail = NULL;
        list->cur_index = HEAD;
        list->current = &(list->head);
        if (thread_safe)
        {
            list->lock = cplus_rwlock_new();
        }
    }
    else
    {
        errno = ENOMEM;
    }

    return list;
exit:
    cplus_llist_delete(list);
    return NULL;
}

static inline bool is_empty(struct linked_list * list)
{
    return (0 == list->count);
}

static inline uint32_t get_size(struct linked_list * list)
{
    return list->count;
}

static inline bool is_sort(struct linked_list * list)
{
    return (get_size(list) == list->sort_count);
}

static void free_node(struct linked_list * list, struct llist_node * node)
{
    if (node)
    {
        if (NULL != list->mempool)
        {
            cplus_mempool_free(list->mempool, node);
        }
        else
        {
            cplus_free(node);
        }
    }
}

static struct llist_node * new_node(struct linked_list * list, void * data)
{
    struct llist_node * node = NULL;

    if (NULL != list->mempool)
    {
        node = (struct llist_node *)cplus_mempool_alloc(list->mempool);
    }
    else
    {
        node = (struct llist_node *)cplus_malloc(sizeof(struct llist_node));
    }

    if (NULL == node)
    {
        errno = ENOMEM;
    }
    else
    {
        node->data = data;
        node->next = NULL;
        node->prev = NULL;
    }
    return node;
}

static inline void * get_data(struct llist_node * node)
{
    return node->data;
}

static void reset_cur_node(struct linked_list * list)
{
    list->cur_index = HEAD;
    list->current = &(list->head);
    return;
}

static int32_t remove_node(struct linked_list * list, struct llist_node * node)
{
    if (list->head == node)
    {
        list->head = node->next;
        if (node->next)
        {
            (node->next)->prev = NULL;
        }
        node->next = NULL;
    }
    else if (list->tail == node)
    {
        list->tail = node->prev;
        if (node->prev)
        {
            (node->prev)->next = NULL;
        }
        node->prev = NULL;
    }
    else
    {
        (node->prev)->next = node->next;
        (node->next)->prev = node->prev;
        node->next = NULL;
        node->prev = NULL;
    }
    list->count--;

    if (is_empty(list))
    {
        list->head = NULL;
        list->tail = NULL;
    }

    reset_cur_node(list);

    return CPLUS_SUCCESS;
}

static void swap_data(struct llist_node * front, struct llist_node * back)
{
    void * temp = NULL;

    if (NULL == front || NULL == back)
    {
        return;
    }
    else
    {
        temp = front->data;
        front->data = back->data;
        back->data = temp;
    }
}

static inline struct llist_node * get_head(struct linked_list * list)
{
    return list->head;
}

static inline struct llist_node * get_tail(struct linked_list * list)
{
    return list->tail;
}

static inline struct llist_node * get_current(struct linked_list * list)
{
    return *(list->current);
}

static struct llist_node * get_next(struct linked_list * list)
{
    if (*(list->current) && (*(list->current))->next)
    {
        list->current = &((*(list->current))->next);
        list->cur_index ++;
        return *(list->current);
    }
    return NULL;
}

static struct llist_node * get_prev(struct linked_list * list)
{
    if (*(list->current) && (*(list->current))->prev)
    {
        list->current = &((*(list->current))->prev);
        list->cur_index --;
        return *(list->current);
    }
    return NULL;
}

static struct llist_node * get_node_of(
    struct linked_list * list
    , int32_t index)
{
    if (index == list->cur_index)
    {
        return *(list->current);
    }
    else if (index > list->cur_index)
    {
        while (index != list->cur_index)
        {
            list->current = &((*(list->current))->next);
            list->cur_index ++;
        }
    }
    else
    {
        while (index != list->cur_index)
        {
            list->current = &((*(list->current))->prev);
            list->cur_index --;
        }
    }

    return *(list->current);
}

static struct llist_node * get_node_if(
    struct linked_list * list
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    list->cur_index = 0;
    list->current = &(list->head);

    while (*(list->current))
    {
        void * data = get_data(*(list->current));
        if (data && !comparator(data, arg))
        {
            return *(list->current);
        }

        if (NULL == (*(list->current))->next)
        {
            break;
        }
        else
        {
            list->current = &((*(list->current))->next);
            list->cur_index ++;
        }
    }

    errno = ENOENT;
    return NULL;
}

static int32_t get_index_if(
    struct linked_list * list
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    list->cur_index = 0;
    list->current = &(list->head);

    while (*(list->current))
    {
        void * data = get_data(*(list->current));
        if (data && !comparator(data, arg))
        {
            return list->cur_index;
        }

        if (NULL == (*(list->current))->next)
        {
            break;
        }
        else
        {
            list->current = &((*(list->current))->next);
            list->cur_index ++;
        }
    }

    errno = ENOENT;
    return CPLUS_FAIL;
}

static struct llist_node * pop_of(struct linked_list * list, int32_t index)
{
    struct llist_node * node = NULL;

    if (is_empty(list))
    {
        errno = ENOENT;
        return NULL;
    }
    else
    {
        if (0 == index)
        {
            node = list->head;
            list->head = node->next;
            if (node->next)
            {
                (node->next)->prev = NULL;
            }
            node->next = NULL;
            reset_cur_node(list);
        }
        else if (get_size(list) == ((uint32_t)(index)))
        {
            node = list->tail;
            list->tail = node->prev;
            if (node->prev)
            {
                (node->prev)->next = NULL;
            }
            node->prev = NULL;
            reset_cur_node(list);
        }
        else
        {
            if ((node = get_node_of(list, index)))
            {
                (node->prev)->next = node->next;
                (node->next)->prev = node->prev;
                node->next = NULL;
                node->prev = NULL;
            }
        }
        list->count--;

        if (is_empty(list))
        {
            list->head = NULL;
            list->tail = NULL;
            reset_cur_node(list);
        }
    }

    return node;
}

static int32_t push_at(
    struct linked_list * list
    , int32_t index
    , struct llist_node * node)
{
    int32_t res = CPLUS_SUCCESS;
    struct llist_node * target = NULL;

    if (is_empty(list))
    {
        list->head = node;
        list->tail = node;
        reset_cur_node(list);
    }
    else
    {
        if (0 == index)
        {
            (list->head)->prev = node;
            node->next = list->head;
            list->head = node;
            reset_cur_node(list);
        }
        else if (get_size(list) == ((uint32_t)(index)))
        {
            (list->tail)->next = node;
            node->prev = list->tail;
            list->tail = node;
            reset_cur_node(list);
        }
        else
        {
            if (!(target = get_node_of(list, index)))
            {
                res = CPLUS_FAIL;
            }
            else
            {
                node->next = target;
                node->prev = target->prev;
                (target->prev)->next = node;
                target->prev = node;
            }
        }
    }

    if (CPLUS_SUCCESS == res)
    {
        list->count ++;
    }

    return res;
}

static struct llist_node * pop_if(
    struct linked_list * list
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    list->cur_index = 0;
    list->current = &(list->head);

    while (!is_empty(list) && *(list->current))
    {
        void * data = get_data(*(list->current));
        if (data && !comparator(data, arg))
        {
            struct llist_node * target = *(list->current);
            remove_node(list, *(list->current));
            return target;
        }

        if (NULL == (*(list->current))->next)
        {
            break;
        }
        else
        {
            list->current = &((*(list->current))->next);
            list->cur_index ++;
        }
    }

    errno = ENOENT;
    return NULL;
}

static void * get_set_if(
    struct linked_list * list
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    struct linked_list * target_list = NULL;
    struct llist_node * node = NULL;

    list->cur_index = 0;
    list->current = &(list->head);

    while (*(list->current))
    {
        void * data = get_data(*(list->current));
        if (data && !comparator(data, arg))
        {
            if (NULL == target_list)
            {
                target_list = (struct linked_list *)llist_initialize_object(0, false);
            }

            if ((node = new_node(target_list, data)))
            {
                push_at(target_list, get_size(target_list), node);
            }
        }

        if (NULL == (*(list->current))->next)
        {
            break;
        }
        else
        {
            list->current = &((*(list->current))->next);
            list->cur_index ++;
        }
    }

    if (NULL == target_list)
    {
        errno = ENOENT;
    }
    return target_list;
}

static int32_t remove_data(
    struct linked_list * list
    , void * data)
{
    list->cur_index = 0;
    list->current = &(list->head);

    while (*(list->current))
    {
        if (data == (void *)((*(list->current))->data))
        {
            struct llist_node * temp = *(list->current);
            remove_node(list, *(list->current));
            free_node(list, temp);
            return CPLUS_SUCCESS;
        }

        if (NULL == (*(list->current))->next)
        {
            break;
        }
        else
        {
            list->current = &((*(list->current))->next);
            list->cur_index ++;
        }
    }
    return CPLUS_FAIL;
}

static void llist_sort(
    struct linked_list * list
    , int32_t (* comparator)(void * data1, void * data2))
{
    struct llist_node * node1 = list->head, * node2 = NULL;
    void * data1 = NULL, * data2 = NULL;
    uint32_t size = get_size(list);

    for (uint32_t i = 0
        ; i < get_size(list) && NULL != node1
        ; i ++, size --)
    {
        node1 = list->head;
        for (uint32_t j = 0
            ; j < size && NULL != node1
            ; j++, node1 = node1->next)
        {
            node2 = node1->next;
            if (NULL == node1 || NULL == node2)
            {
                break;
            }
            else
            {
                data1 = get_data(node1);
                data2 = get_data(node2);

                if (0 < comparator(data1, data2))
                {
                    swap_data(node1, node2);
                }
            }
        }
        list->sort_count++;
    }
    return;
}

uint32_t cplus_llist_get_size(cplus_llist obj)
{
    int32_t count = 0;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    count = get_size(list);
    cplus_lock_unlock(list->lock);

    return count;
}

void * cplus_llist_get_of(cplus_llist obj, int32_t index)
{
    void * target = NULL;
    struct llist_node * node = NULL;
    struct linked_list * list = (struct linked_list *)(obj);

    CHECK_OBJECT_TYPE(obj);
    CHECK_IN_INTERVAL(index, 0, (int32_t)(cplus_llist_get_size(obj) - 1), NULL);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_node_of(list, index)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

int32_t cplus_llist_get_index_if(
    cplus_llist obj
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    int32_t res = CPLUS_FAIL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(comparator, CPLUS_FAIL);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    res = get_index_if(list, comparator, arg);
    cplus_lock_unlock(list->lock);

    return res;
}

void * cplus_llist_pop_of(cplus_llist obj, int32_t index)
{
    void * target = NULL;
    struct llist_node * node = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_IN_INTERVAL(index, 0, (int32_t)(cplus_llist_get_size(obj)), NULL);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = pop_of(list, index)))
    {
        target = get_data(node);
        free_node(list, node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

int32_t cplus_llist_push_at(
    cplus_llist obj
    , int32_t index
    , void * data)
{
    int32_t res = CPLUS_FAIL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(data, CPLUS_FAIL);
    CHECK_IN_INTERVAL(index, 0, (int32_t)(cplus_llist_get_size(obj)), CPLUS_FAIL);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = new_node(list, data)))
    {
        res = push_at(list, index, node);
    }
    cplus_lock_unlock(list->lock);

    return res;
}

void * cplus_llist_pop_front(cplus_llist obj)
{
    return cplus_llist_pop_of(obj, 0);
}

void * cplus_llist_pop_back(cplus_llist obj)
{
    return cplus_llist_pop_of(obj, cplus_llist_get_size(obj));
}

int32_t cplus_llist_push_front(cplus_llist obj, void * data)
{
    return cplus_llist_push_at(obj, 0, data);
}

int32_t cplus_llist_push_back(cplus_llist obj, void * data)
{
    return cplus_llist_push_at(obj, cplus_llist_get_size(obj), data);
}

int32_t cplus_llist_clear(cplus_llist obj)
{
    struct llist_node * node = NULL, * temp = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    node = list->head;
    while (node)
    {
        temp = node;
        node = node->next;
        free_node(list, temp);
    }

    list->count = 0;
    list->sort_count = 0;
    list->head = NULL;
    list->tail = NULL;
    reset_cur_node(list);
    cplus_lock_unlock(list->lock);

    return CPLUS_SUCCESS;
}

int32_t cplus_llist_delete(cplus_llist obj)
{
    int32_t res = CPLUS_FAIL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);

    res = cplus_llist_clear(obj);
    if (CPLUS_SUCCESS == res)
    {
        if (list->lock)
        {
            cplus_rwlock_delete(list->lock);
        }
        if (list->mempool)
        {
            cplus_mempool_delete(list->mempool);
        }
        cplus_free(list);
    }

    return res;
}

cplus_llist cplus_llist_get_set_if(
    cplus_llist obj
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    cplus_llist target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(comparator, NULL);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    target = get_set_if(list, comparator, arg);
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_pop_if(
    cplus_llist obj
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    void * target = NULL;
    struct llist_node * node = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(comparator, NULL);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    node = pop_if(list, comparator, arg);
    if (node)
    {
        target = get_data(node);
        free_node(list, node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_get_if(
    cplus_llist obj
    , int32_t (* comparator)(void * data, void * arg)
    , void * arg)
{
    void * target = NULL;
    struct llist_node * node = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(comparator, NULL);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_node_if(list, comparator, arg)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

bool cplus_llist_is_consist(
    cplus_llist obj1
    , cplus_llist obj2
    , int32_t (* comparator)(void * data1, void * data2))
{
    CHECK_OBJECT_TYPE(obj1);
    CHECK_OBJECT_TYPE(obj2);
    CHECK_NOT_NULL(comparator, false);

    if (cplus_llist_get_size(obj1)
        != cplus_llist_get_size(obj2))
    {
        return false;
    }

    for (uint32_t idx = 0; idx < cplus_llist_get_size(obj1); idx ++)
    {
        if (0 != comparator(
            cplus_llist_get_of(obj1, idx)
            , cplus_llist_get_of(obj2, idx)))
        {
            return false;
        }
    }

    return true;
}

int32_t cplus_llist_delete_data(cplus_llist obj, void * data)
{
    int32_t res = CPLUS_FAIL;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(data, CPLUS_FAIL);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    res = remove_data(list, data);
    cplus_lock_unlock(list->lock);

    return res;
}

int32_t cplus_llist_sort(
    cplus_llist obj
    , int32_t (* comparator)(void * data1, void * data2))
{
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(comparator, CPLUS_FAIL);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if (false == is_sort(list))
    {
        llist_sort(list, comparator);
    }
    cplus_lock_unlock(list->lock);

    return (true == is_sort(list))? CPLUS_SUCCESS: CPLUS_FAIL;
}

bool cplus_llist_is_sort(cplus_llist obj)
{
    bool issort = false;
    struct linked_list * list = (struct linked_list *)(obj);
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    issort = is_sort(list);
    cplus_lock_unlock(list->lock);

    return issort;
}

void * cplus_llist_get_head(cplus_llist obj)
{
    void * target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_head(list)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_get_tail(cplus_llist obj)
{
    void * target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_tail(list)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_get_current(cplus_llist obj)
{
    void * target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_shlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_current(list)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_get_next(cplus_llist obj)
{
    void * target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_next(list)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_get_prev(cplus_llist obj)
{
    void * target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_prev(list)))
    {
        target = get_data(node);
    }
    cplus_lock_unlock(list->lock);

    return target;
}

void * cplus_llist_get_cycling_next(cplus_llist obj)
{
    void * target = NULL;
    struct linked_list * list = (struct linked_list *)(obj);
    struct llist_node * node = NULL;
    CHECK_OBJECT_TYPE(obj);

    cplus_lock_exlock(list->lock, CPLUS_INFINITE_TIMEOUT);
    if ((node = get_current(list)))
    {
        target = get_data(node);
        if (!(get_next(list)))
        {
            reset_cur_node(list);
        }
    }
    cplus_lock_unlock(list->lock);

    return target;
}

cplus_llist cplus_llist_prev_new(uint32_t max_count)
{
    CHECK_IF(MAX_NODE_COUNT < max_count, NULL);
    return llist_initialize_object(max_count, false);
}

cplus_llist cplus_llist_new(void)
{
    return llist_initialize_object(0, false);
}

cplus_llist cplus_llist_prev_new_s(uint32_t max_count)
{
    CHECK_IF(MAX_NODE_COUNT < max_count, NULL);
    return llist_initialize_object(max_count, true);
}

cplus_llist cplus_llist_new_s(void)
{
    return llist_initialize_object(0, true);
}

bool cplus_llist_check(cplus_object obj)
{
    return (obj && (GET_OBJECT_TYPE(obj) == OBJ_TYPE));
}

static int find_node_by_key(void * data1, void * data2)
{
    char * data_key = cplus_data_get_key((cplus_data)(data1));
    char * key = (char *)(data2);

    if (NULL == data_key OR NULL == key)
    {
        return 1;
    }
    return strcmp(data_key, key);
}

cplus_data cplus_llist_add_data(
    cplus_llist obj
    , CPLUS_DATA_TYPE type
    , char * key
    , void * value1
    , void * value2)
{
    cplus_data data = NULL;
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(key, NULL);
    CHECK_NOT_NULL(value1, NULL);

    data = cplus_llist_get_if(obj, find_node_by_key, (void *)(key));
    if (!cplus_data_check(data))
    {
        if ((data = cplus_data_new_ex(type, value1, value2, strlen(key), key)))
        {
            cplus_llist_push_back(obj, data);
        }
    }
    else
    {
        cplus_data_set_value(data, value1, value2);
    }
    return data;
}

cplus_data cplus_llist_find_data(cplus_llist obj, char * key)
{
    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(key, NULL);
    return cplus_llist_get_if(obj, find_node_by_key, (void *)(key));
}

cplus_data cplus_llist_update_data(cplus_llist obj, char * key, cplus_data data)
{
    cplus_data target = NULL;

    CHECK_OBJECT_TYPE(obj);
    CHECK_NOT_NULL(key, NULL);
    CHECK_IF_NOT(cplus_data_check(data), NULL);

    if (NULL != (target = cplus_llist_get_if(obj, find_node_by_key, (void *)(key))))
    {
        if (CPLUS_SUCCESS != cplus_data_clone_value(target, data))
        {
            return NULL;
        }
    }
    if (CPLUS_SUCCESS == cplus_data_set_key(data, strlen(key), key)
        && CPLUS_SUCCESS == cplus_llist_push_back(obj, data))
    {
        target = data;
    }
    return target;
}

int32_t cplus_llist_remove_data(cplus_llist obj, char * key)
{
    cplus_data data = NULL;
    CHECK_NOT_NULL(obj, CPLUS_FAIL);
    CHECK_NOT_NULL(key, CPLUS_FAIL);

    if ((data = (cplus_data)cplus_llist_pop_if(obj
        , find_node_by_key
        , (void *)(key))))
    {
        cplus_data_delete(data);
    }
    return CPLUS_SUCCESS;
}

cplus_data cplus_llist_add_data_bool(cplus_llist obj, char * key, bool value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_BOOL, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_int8(cplus_llist obj, char * key, int8_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_INT8, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_int16(cplus_llist obj, char * key, int16_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_INT16, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_int32(cplus_llist obj, char * key, int32_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_INT32, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_int64(cplus_llist obj, char * key, int64_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_INT64, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_uint8(cplus_llist obj, char * key, uint8_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_UINT8, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_uint16(cplus_llist obj, char * key, uint16_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_UINT16, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_uint32(cplus_llist obj, char * key, uint32_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_UINT32, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_uint64(cplus_llist obj, char * key, uint64_t value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_UINT64, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_float(cplus_llist obj, char * key, float value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_FLOAT, key, &(value), NULL);
}

cplus_data ccplus_llist_add_data_double(cplus_llist obj, char * key, double value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_DOUBLE, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_pointer(cplus_llist obj, char * key, void * value)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_POINTER, key, &(value), NULL);
}

cplus_data cplus_llist_add_data_string(cplus_llist obj, char * key, char * str_bufs, uint32_t str_len)
{
    return cplus_llist_add_data(obj, CPLUS_DATA_TYPE_STRING, key, &(str_len), str_bufs);
}

#ifdef __CPLUS_UNITTEST__
static int32_t num0 = 0, num1 = 1, num2 = 2, num3 = 3, num4 = 4, num5 = 5;

int32_t find_num(void * data, void * arg)
{
    if (*((int32_t *)data) == *((int32_t *)arg))
    {
        return 0;
    }
    return 1;
}

int32_t compare_num(void * data1, void * data2)
{
    if (*((int32_t *)data1) == *((int32_t *)data2))
    {
        return 0;
    }
    return 1;
}

CPLUS_UNIT_TEST(cplus_llist_push_front, sequence)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_front, bad_parameter)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_back, sequence)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_back, bad_parameter)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_back(list, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_at, push_randomly)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 2, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_pop_of, pop_randomly)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_of(list, 0)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_of(list, 3)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_index_if, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_index_if(list, find_num, &num0));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_index_if(list, find_num, &num1));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_index_if(list, find_num, &num2));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_index_if(list, find_num, &num3));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_index_if(list, find_num, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_index_if, bad_parameter)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, NULL, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, find_num, &num5));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_front, sequence_with_mempool)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_prev_new(1024))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
}

CPLUS_UNIT_TEST(cplus_llist_push_front, bad_parameter_with_mempool)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_prev_new(1024))));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_LLIST_FOREACH, sequence)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));

    int * data = NULL, value = 4;
    CPLUS_LLIST_FOREACH(list, data)
    {
        UNITTEST_EXPECT_EQ(value, *data);
        value --;
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_set_if, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(15, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(16, cplus_mgr_report());

    cplus_llist list_0 = NULL, list_1 = NULL, list_2 = NULL;
    cplus_llist list_3 = NULL, list_4 = NULL, list_5 = NULL;

    UNITTEST_EXPECT_NE(true, (NULL != (list_0 = cplus_llist_get_set_if(list, find_num, &num0))));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(true, (NULL != (list_1 = cplus_llist_get_set_if(list, find_num, &num1))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_2 = cplus_llist_get_set_if(list, find_num, &num2))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_3 = cplus_llist_get_set_if(list, find_num, &num3))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_4 = cplus_llist_get_set_if(list, find_num, &num4))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_5 = cplus_llist_get_set_if(list, find_num, &num5))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_1));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_2));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list_1)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_1));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_2));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_2));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_5));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_pop_if, functionity)
{
    int32_t * data = NULL;
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
    UNITTEST_EXPECT_EQ(0, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
    UNITTEST_EXPECT_EQ(1, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
    UNITTEST_EXPECT_EQ(2, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
    UNITTEST_EXPECT_EQ(3, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
    UNITTEST_EXPECT_EQ(4, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_pop_if, bad_parameter)
{
    int32_t * data = NULL;
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(true, (NULL == (data = (int32_t *)cplus_llist_pop_if(list, NULL, NULL))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num5))));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_if, functionity)
{
    int32_t * data = NULL;
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num0))));
    UNITTEST_EXPECT_EQ(0, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num1))));
    UNITTEST_EXPECT_EQ(1, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num2))));
    UNITTEST_EXPECT_EQ(2, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num3))));
    UNITTEST_EXPECT_EQ(3, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num4))));
    UNITTEST_EXPECT_EQ(4, *data);
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_is_consist, functionity)
{
    cplus_llist list1 = NULL, list2 = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
    UNITTEST_EXPECT_EQ(false, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));

    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
    UNITTEST_EXPECT_EQ(false, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));


    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
    UNITTEST_EXPECT_EQ(false, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));


    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
    UNITTEST_EXPECT_EQ(true, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));

    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_front_s, sequence)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_front_s, bad_parameter)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_back_s, sequence)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_back_s, bad_parameter)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_back(list, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_at_s, push_randomly)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 0, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 1, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_at(list, 2, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_pop_of_s, pop_randomly)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_of(list, 0)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_of(list, 3)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_index_if_s, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_index_if(list, find_num, &num0));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_index_if(list, find_num, &num1));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_index_if(list, find_num, &num2));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_index_if(list, find_num, &num3));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_index_if(list, find_num, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_index_if_s, bad_parameter)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, NULL, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_get_index_if(list, find_num, &num5));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_front_s, sequence_with_mempool)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_prev_new_s(1024))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
}

CPLUS_UNIT_TEST(cplus_llist_push_front_s, bad_parameter_with_mempool)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_prev_new_s(1024))));
    UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_llist_push_front(list, NULL));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_LLIST_FOREACH_s, sequence)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));

    int * data = NULL, value = 4;
    CPLUS_LLIST_FOREACH(list, data)
    {
        UNITTEST_EXPECT_EQ(value, *data);
        value --;
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_set_if_s, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num5));
    UNITTEST_EXPECT_EQ(15, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(17, cplus_mgr_report());

    cplus_llist list_0 = NULL, list_1 = NULL, list_2 = NULL;
    cplus_llist list_3 = NULL, list_4 = NULL, list_5 = NULL;

    UNITTEST_EXPECT_NE(true, (NULL != (list_0 = cplus_llist_get_set_if(list, find_num, &num0))));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(true, (NULL != (list_1 = cplus_llist_get_set_if(list, find_num, &num1))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_2 = cplus_llist_get_set_if(list, find_num, &num2))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_3 = cplus_llist_get_set_if(list, find_num, &num3))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_4 = cplus_llist_get_set_if(list, find_num, &num4))));
    UNITTEST_EXPECT_EQ(true, (NULL != (list_5 = cplus_llist_get_set_if(list, find_num, &num5))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_1));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_2));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list_1)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_1));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_2));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list_2)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_2));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list_3)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_3));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list_4)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_4));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(5, *((int32_t *)cplus_llist_pop_back(list_5)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list_5));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list_5));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_pop_if_s, functionity)
{
    int32_t * data = NULL;
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
    UNITTEST_EXPECT_EQ(0, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(4, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num0)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
    UNITTEST_EXPECT_EQ(1, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num1)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
    UNITTEST_EXPECT_EQ(2, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(2, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num2)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
    UNITTEST_EXPECT_EQ(3, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(1, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num3)));
    UNITTEST_EXPECT_EQ(true, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
    UNITTEST_EXPECT_EQ(4, (int32_t)(*data));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(false, NULL != (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_pop_if_s, bad_parameter)
{
    int32_t * data = NULL;
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(true, (NULL == (data = (int32_t *)cplus_llist_pop_if(list, NULL, NULL))));
    UNITTEST_EXPECT_EQ(EINVAL, errno);
    UNITTEST_EXPECT_EQ(true, (NULL == (data = (int32_t *)cplus_llist_pop_if(list, find_num, &num5))));
    UNITTEST_EXPECT_EQ(ENOENT, errno);
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_if_s, functionity)
{
    int32_t * data = NULL;
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(7, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num0))));
    UNITTEST_EXPECT_EQ(0, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num1))));
    UNITTEST_EXPECT_EQ(1, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num2))));
    UNITTEST_EXPECT_EQ(2, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num3))));
    UNITTEST_EXPECT_EQ(3, *data);
    UNITTEST_EXPECT_EQ(true, NULL != (data = ((int32_t *)cplus_llist_get_if(list, find_num, &num4))));
    UNITTEST_EXPECT_EQ(4, *data);
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_is_consist_s, functionity)
{
    cplus_llist list1 = NULL, list2 = NULL;

    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
    UNITTEST_EXPECT_EQ(false, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));

    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
    UNITTEST_EXPECT_EQ(false, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));


    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
    UNITTEST_EXPECT_EQ(false, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));


    UNITTEST_EXPECT_EQ(true, (NULL != (list1 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(true, (NULL != (list2 = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list1, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list2, &num4));
    UNITTEST_EXPECT_EQ(true, cplus_llist_is_consist(list1, list2, compare_num));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list2));

    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());

}

CPLUS_UNIT_TEST(cplus_llist_delete_data, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num3));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_back(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_delete_data_s, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new_s())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num4));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete_data(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

int32_t ascendant(void * i1, void * i2)
{
    return *((int32_t *)i1) - *((int32_t *)i2);
}

int32_t deascendant(void * i1, void * i2)
{
    return *((int32_t *)i2) - *((int32_t *)i1);
}

CPLUS_UNIT_TEST(cplus_llist_sort, ascending)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, ascendant));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_sort, deascending)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, deascendant));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_is_sort(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_sort_s, ascending)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, ascendant));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_is_sort(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_sort_s, deascending)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num4));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_front(list, &num1));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_sort(list, deascendant));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_push_back, pop_then_push)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_pop_front(list)));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_of(list, 0)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_of(list, 1)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_of(list, 2)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_of(list, 3)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_of(list, 4)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_clear(list));
    UNITTEST_EXPECT_EQ(0, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_add_data, functionity)
{
    cplus_llist list = NULL;
    cplus_data data = NULL;
    int32_t i32_5 = 5, i32_6 = 6, i32_7 = 7, i32_8 = 8, i32_9 = 9;
    char * str1 = (char *)("Hello world"), * str2 = (char *)("AaBbCcDd");
    uint32_t str1_len = strlen(str1);

    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data_int32(list, (char *)("int_0"), 0))));
    UNITTEST_EXPECT_EQ(0, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data_int32(list, (char *)("int_1"), 1))));
    UNITTEST_EXPECT_EQ(1, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data_int32(list, (char *)("int_2"), 2))));
    UNITTEST_EXPECT_EQ(2, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data_int32(list, (char *)("int_3"), 3))));
    UNITTEST_EXPECT_EQ(3, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data_int32(list, (char *)("int_4"), 4))));
    UNITTEST_EXPECT_EQ(4, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_0"))));
    UNITTEST_EXPECT_EQ(1, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_1"))));
    UNITTEST_EXPECT_EQ(2, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_2"))));
    UNITTEST_EXPECT_EQ(3, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_3"))));
    UNITTEST_EXPECT_EQ(4, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_4"))));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data(
            list, CPLUS_DATA_TYPE_INT32, (char *)("int_0"), &i32_5, NULL))));
    UNITTEST_EXPECT_EQ(5, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data(
            list, CPLUS_DATA_TYPE_INT32, (char *)("int_1"), &i32_6, NULL))));
    UNITTEST_EXPECT_EQ(6, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data(
            list, CPLUS_DATA_TYPE_INT32, (char *)("int_2"), &i32_7, NULL))));
    UNITTEST_EXPECT_EQ(7, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data(
            list, CPLUS_DATA_TYPE_INT32, (char *)("int_3"), &i32_8, NULL))));
    UNITTEST_EXPECT_EQ(8, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data(
            list, CPLUS_DATA_TYPE_INT32, (char *)("int_4"), &i32_9, NULL))));
    UNITTEST_EXPECT_EQ(9, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(5, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_0"))));
    UNITTEST_EXPECT_EQ(6, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_1"))));
    UNITTEST_EXPECT_EQ(7, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_2"))));
    UNITTEST_EXPECT_EQ(8, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_3"))));
    UNITTEST_EXPECT_EQ(9, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_4"))));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_remove_data(list, (char *)("int_1")));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_remove_data(list, (char *)("int_3")));
    UNITTEST_EXPECT_EQ(true, (NULL == cplus_llist_find_data(list, (char *)("int_1"))));
    UNITTEST_EXPECT_EQ(true, (NULL == cplus_llist_find_data(list, (char *)("int_3"))));
    UNITTEST_EXPECT_EQ(3, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data(
            list, CPLUS_DATA_TYPE_STRING, (char *)("str1"), &str1_len, (void *)str1))));
    UNITTEST_EXPECT_EQ(0, strcmp(
        str1
        , cplus_data_get_string(data)));
    UNITTEST_EXPECT_EQ(
        true
        , (NULL != (data = cplus_llist_add_data_string(
            list, (char *)("str2"), str2, strlen(str2)))));
    UNITTEST_EXPECT_EQ(0, strcmp(
        str2
        , cplus_data_get_string(data)));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, strcmp(
        str1
        , cplus_data_get_string(cplus_llist_find_data(list, (char *)("str1")))));
    UNITTEST_EXPECT_EQ(0, strcmp(
        str2
        , cplus_data_get_string(cplus_llist_find_data(list, (char *)("str2")))));
    UNITTEST_EXPECT_EQ(strlen(str1), strlen(cplus_data_get_string(cplus_llist_find_data(list, (char *)("str1")))));
    UNITTEST_EXPECT_EQ(strlen(str2), strlen(cplus_data_get_string(cplus_llist_find_data(list, (char *)("str2")))));
    UNITTEST_EXPECT_EQ(true, NULL != (data = cplus_data_new_int32(0x1234)));
    UNITTEST_EXPECT_EQ(true, NULL != cplus_llist_update_data(list, (char *)("int_0"), data));
    UNITTEST_EXPECT_EQ(0x1234, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_0"))));
    UNITTEST_EXPECT_EQ(true, NULL != (data = cplus_data_new_int32(0x4321)));
    UNITTEST_EXPECT_EQ(0x4321, cplus_data_get_int32(data));
    UNITTEST_EXPECT_EQ(true, NULL != cplus_llist_update_data(list, (char *)("int_5"), data));
    UNITTEST_EXPECT_EQ(0x4321, cplus_data_get_int32(cplus_llist_find_data(list, (char *)("int_5"))));
    CPLUS_LLIST_FOREACH(list, data)
    {
        UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_next, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_head(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_tail(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_next(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_next(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_next(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_next(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_prev(list)));
    UNITTEST_EXPECT_EQ(3, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_prev(list)));
    UNITTEST_EXPECT_EQ(2, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_prev(list)));
    UNITTEST_EXPECT_EQ(1, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_prev(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_current(list)));
    UNITTEST_EXPECT_EQ(0, *((int32_t *)cplus_llist_get_head(list)));
    UNITTEST_EXPECT_EQ(4, *((int32_t *)cplus_llist_get_tail(list)));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_LLIST_FOREACH, functionity)
{
    cplus_llist list = NULL;
    int32_t * target = 0, idx = 0;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    CPLUS_LLIST_FOREACH(list, target)
    {
        UNITTEST_EXPECT_EQ(idx, *(target));
        UNITTEST_EXPECT_EQ(idx, *((int32_t *)cplus_llist_get_current(list)));
        idx++;
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_llist_get_cycling_next, functionity)
{
    cplus_llist list = NULL;
    UNITTEST_EXPECT_EQ(true, (NULL != (list = cplus_llist_new())));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num0));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num1));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num2));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num3));
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(list, &num4));
    UNITTEST_EXPECT_EQ(5, cplus_llist_get_size(list));
    for (int32_t idx = 0; idx < 423; idx ++)
    {
        UNITTEST_EXPECT_EQ((idx % 5), *((int32_t *)cplus_llist_get_cycling_next(list)));
    }
    UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_delete(list));
    UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

void unittest_llist(void)
{
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front, sequence);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_back, sequence);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_back, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_at, push_randomly);
    UNITTEST_ADD_TESTCASE(cplus_llist_pop_of, pop_randomly);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_index_if, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_index_if, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front, sequence_with_mempool);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front, bad_parameter_with_mempool);
    UNITTEST_ADD_TESTCASE(CPLUS_LLIST_FOREACH, sequence);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_set_if, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_pop_if, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_pop_if, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_if, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_is_consist, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front_s, sequence);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front_s, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_back_s, sequence);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_back_s, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_at_s, push_randomly);
    UNITTEST_ADD_TESTCASE(cplus_llist_pop_of_s, pop_randomly);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_index_if_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_index_if_s, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front_s, sequence_with_mempool);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_front_s, bad_parameter_with_mempool);
    UNITTEST_ADD_TESTCASE(CPLUS_LLIST_FOREACH_s, sequence);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_set_if_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_pop_if_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_pop_if_s, bad_parameter);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_if_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_is_consist_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_delete_data, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_delete_data_s, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_sort, ascending);
    UNITTEST_ADD_TESTCASE(cplus_llist_sort, deascending);
    UNITTEST_ADD_TESTCASE(cplus_llist_sort_s, ascending);
    UNITTEST_ADD_TESTCASE(cplus_llist_sort_s, deascending);
    UNITTEST_ADD_TESTCASE(cplus_llist_push_back, pop_then_push);
    UNITTEST_ADD_TESTCASE(cplus_llist_add_data, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_next, functionity);
    UNITTEST_ADD_TESTCASE(CPLUS_LLIST_FOREACH, functionity);
    UNITTEST_ADD_TESTCASE(cplus_llist_get_cycling_next, functionity);
}

#endif