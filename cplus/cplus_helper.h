#ifndef __CPLUS_HELPER_H__
#define __CPLUS_HELPER_H__
#include "cplus_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_TRUE_ON_BUILDING(E) (sizeof(char[1 - 2 * !!(E)]) - 1)

#define CPLUS_OFFSETOF(TYPE, MEMBER) ((size_t)(&(((TYPE *)0)->MEMBER)))

#define CPLUS_CONTAINER_OF(PTR, TYPE, MEMBER) \
    ({ \
        void * memptr = (void *)PTR; \
        (TYPE *)((char *)(memptr) - (char *)(CPLUS_OFFSETOF(TYPE, MEMBER))); \
    })

#define CPLUS_INITIALIZE_STRUCT_POINTER(PTR) cplus_mem_set(PTR, 0x00, sizeof(typeof(* (PTR))))

#define CPLUS_MAX(A, B) \
    ({ typeof(A) _A = (A); typeof(B) _B = (B); _A > _B ? _A : _B; })

#define CPLUS_MIN(A, B) \
    ({ typeof(A) _A = (A); typeof(B) _B = (B); _A < _B ? _A : _B; })

#define CPLUS_DIFF(A, B) \
    ({ typeof(A) _A = (A); typeof(B) _B = (B); _A > _B ? _A - _B : _B - _A; })

#define LAP_FULL(...) ({ __VA_ARGS__ })
#define LAP3(...) _LAP3 __VA_ARGS__)
#define _LAP3(...) __VA_ARGS__ __CPLUS_LAMBDA LAP2(
#define LAP2(...) _LAP2 __VA_ARGS__)
#define _LAP2(...) (__VA_ARGS__) LAP1(
#define LAP1(...) __VA_ARGS__ __CPLUS_LAMBDA;

/* AMI CICD don`t like it */
/* CPLUS_LAMBDA((Return-Value Type)(Parameter List){Lambda Body}) */
#define CPLUS_LAMBDA(...) LAP_FULL(LAP3(__VA_ARGS__))

// #define CPLUS_TYPE_CHECK_CONSIST(A, B) __builtin_types_compatible_p(typeof(A), typeof(B))
// #define CPLUS_ARRAY_CHECK(ARRAY) BUILD_BUG_ON_ZERO(CPLUS_TYPE_CHECK_CONSIST((ARRAY), &(ARRAY)[0]))
#define CPLUS_GET_ARRAY_SIZE(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define CPLUS_SET_MODE(VALUE, MODE) (VALUE |= MODE)
#define CPLUS_TEST_MODE(VALUE, MODE) ({(VALUE & MODE);})
#define CPLUS_SET_BIT(VALUE, SHIFT) (VALUE |= (0x1UL << SHIFT))
#define CPLUS_CLEAR_BIT(VALUE, SHIFT) (VALUE &= ~(0x1UL << SHIFT))
#define CPLUS_TEST_BIT(VALUE, SHIFT) ({(*(VALUE) >> SHIFT) & 0x1UL;})

#ifdef __cplusplus
}
#endif
#endif //__CPLUS_HELPER_H__

