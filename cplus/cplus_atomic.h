#ifndef __CPLUS_ATOMIC_H__
#define __CPLUS_ATOMIC_H__

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#ifdef __cplusplus
extern "C" {
#endif

#if (40700 <= GCC_VERSION)
#define cplus_atomic_read(ptr) ({ __atomic_load_n(ptr, __ATOMIC_SEQ_CST); })
#define cplus_atomic_write(ptr, value) ({ __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_exchange(ptr, value) ({ __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_add(ptr, value) ({ __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_and(ptr, value) ({ __atomic_and_fetch(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_xor(ptr, value) ({ __atomic_xor_fetch(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_or(ptr, value) ({ __atomic_or_fetch(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_nand(ptr, value) ({ __atomic_nand_fetch(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_fetch_add(ptr, value) ({ __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_fetch_and(ptr, value) ({ __atomic_fetch_and(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_fetch_xor(ptr, value) ({ __atomic_fetch_xor(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_fetch_or(ptr, value) ({ __atomic_fetch_or(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_fetch_nand(ptr, value) ({ __atomic_fetch_nand(ptr, value, __ATOMIC_SEQ_CST); })
#define cplus_atomic_compare_exchange(ptr, expect_ptr, value_ptr) \
    ({ __atomic_compare_exchange(ptr, expect_ptr, value_ptr, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); })
#elif (40603 <= GCC_VERSION)
#define cplus_atomic_read(ptr) ({ __sync_add_and_fetch(ptr, 0); })
#define cplus_atomic_write(ptr, value) ({ __sync_val_compare_and_swap(ptr, *((typeof(ptr) *)ptr), value); })
#define cplus_atomic_exchange(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_add(ptr, value) ({ __sync_add_and_fetch(ptr, value); })
#define cplus_atomic_and(ptr, value) ({ __sync_and_and_fetch(ptr, value); })
#define cplus_atomic_xor(ptr, value) ({ __sync_xor_and_fetch(ptr, value); })
#define cplus_atomic_or(ptr, value) ({ __sync_or_and_fetch(ptr, value); })
#define cplus_atomic_nand(ptr, value) ({ __sync_nand_and_fetch(ptr, value); })
#define cplus_atomic_fetch_add(ptr, value) ({ __sync_fetch_and_add(ptr, value); })
#define cplus_atomic_fetch_and(ptr, value) ({ __sync_fetch_and_and(ptr, value); })
#define cplus_atomic_fetch_xor(ptr, value) ({ __sync_fetch_and_xor(ptr, value); })
#define cplus_atomic_fetch_or(ptr, value) ({ __sync_fetch_and_or(ptr, value); })
#define cplus_atomic_fetch_nand(ptr, value) ({ __sync_fetch_and_nand(ptr, value); })
#define cplus_atomic_compare_exchange(ptr, expect_ptr, value_ptr) \
    ({ __sync_bool_compare_and_swap(ptr, *((typeof(expect_ptr) *)expect_ptr), *((typeof(value_ptr) *)value_ptr)); })
#else
#define cplus_atomic_read(ptr) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_write(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_exchange(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_add(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_and(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_xor(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_or(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_nand(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_fetch_add(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_fetch_and(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_fetch_xor(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_fetch_or(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_fetch_nand(ptr, value) ({ errno = ENOTSUP; ENOTSUP; })
#define cplus_atomic_compare_exchange(ptr, expect, value) ({ errno = ENOTSUP; ENOTSUP; })
#endif

void * cplus_atomic_read_ptr(void ** ptr);
void cplus_atomic_write_ptr(void ** ptr, void * value);
void * cplus_atomic_exchange_ptr(void ** ptr, void * value);

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_ATOMIC_H__