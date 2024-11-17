#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include "cplus_typedef.h"
#include "cplus_helper.h"
#include "cplus_sys.h"
#include "cplus_atomic.h"

#if !defined(__GNUC__)
#   define __GNUC__ 0
#endif

#if !defined(__GNUC_MINOR__)
#   define __GNUC_MINOR__ 0
#endif

#if !defined(__GNUC_PATCHLEVEL__)
#   define __GNUC_PATCHLEVEL__ 0
#endif

#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if !defined(__STDC__)
#   define __STDC__ 0
#endif

#if !defined(__STDC_VERSION__)
#   define __STDC_VERSION__ 0
#endif

#ifdef __arm__
#   define REG_LD "%lld"
#   define REG_LU "%llu"
#else
#   define REG_LD "%ld"
#   define REG_LU "%lu"
#endif

/* constant variable */
#define OBJ_NONE 0xAC00
#define GB 1073741824
#define INVALID_FD -1
#define INVALID_SOCKET -1

/* build operator */
#define UNUSED_PARAM(PARAM) (void)(PARAM)

/* logical operator */
#define AND &&
#define OR ||
#define NOT !

/* color font */
#define RED(str) "\x1B[31m" str "\033[0m"
#define GREEN(str) "\x1B[32m" str "\033[0m"
#define YELLOW(str) "\x1B[33m" str "\033[0m"
#define BLUE(str) "\x1B[36m" str "\033[0m"
#define PURPLE(str) "\x1B[35m" str "\033[0m"
#define CHANGE_COLOR_RED() printf("\x1B[31m")
#define CHANGE_COLOR_GREEN() printf("\x1B[32m")
#define CHANGE_COLOR_YELLOW() printf("\x1B[33m")
#define CHANGE_COLOR_BLUE() printf("\x1B[36m")
#define CHANGE_COLOR_PURPLE() printf("\x1B[35m")
#define RECOVER_COLOR() printf("\033[0m")

/* condition statement */
#define IF_DO(CONDITION, ACTION_FN) if (CONDITION) { ACTION_FN; }
#define IF_THEN_RETURN(CONDITION, VALUE) if (CONDITION) { errno = EINVAL; return VALUE; }
#define IF_THEN_BREAK(CONDITION) if (CONDITION) { break; }
#define CHECK_IF(CONDITION, VALUE) IF_THEN_RETURN(CONDITION, VALUE)
#define CHECK_IF_NOT(CONDITION, VALUE) IF_THEN_RETURN(!(CONDITION), VALUE)
#define CHECK_GT_ZERO(PARAMETER, VALUE) IF_THEN_RETURN(0 >= PARAMETER, VALUE)
#define CHECK_NOT_NULL(PARAMETER, VALUE) IF_THEN_RETURN(CPLUS_NULL == PARAMETER, VALUE)
#define CHECK_IN_INTERVAL(PARAMETER, MIN, MAX, VALUE) IF_THEN_RETURN((MIN > PARAMETER || PARAMETER > MAX), VALUE)
#define CHECK_OBJECT_NOT_NULL(OBJECT) assert((CPLUS_NULL != OBJECT))
#define CHECK_OBJECT_TYPE(OBJECT) assert((CPLUS_NULL != OBJECT) && (OBJ_TYPE == ((uint16_t *)(OBJECT))[0]))
#define CHECK_OBJECT_TYPE_EX(OBJECT, TYPE) assert((CPLUS_NULL != OBJECT) && ((TYPE) == ((uint16_t *)(OBJECT))[0]))
#define CHECK_OBJECT_TYPE_BOTH(OBJECT, TYPE1, TYPE2) assert((CPLUS_NULL != OBJECT) && (((TYPE1) == ((uint16_t *)(OBJECT))[0]) || ((TYPE2) == ((uint16_t *)(OBJECT))[0])))
#define GET_OBJECT_TYPE(OBJECT) (((uint16_t *)OBJECT)[0])

/* Helper */
#define SAFE_FREE(PTR, RELEASE_FN) if (CPLUS_NULL != PTR) { RELEASE_FN(PTR); PTR = CPLUS_NULL; }
#define TRACE_CODE() printf("[%s](%d)\n", __FUNCTION__, __LINE__)

enum HDLR_CLASS
{
    CORE = 0x00,
    SYS = 0x10,
    DS = 0x20,
    ALG = 0x30,
    CTRL = 0x40,
    IO = 0x50,
    HELPER = 0x60,
};

enum INIT_MODE
{
    INIT_NONE = 0,
    INIT_CREATE,
    INIT_OPEN,
    INIT_HYBRID,
};

#ifdef __CPLUS_UNITTEST__
#include <sys/wait.h>

extern int64_t retval_testfunc;
extern int32_t total_failed_count;

#define CPLUS_UNIT_TEST(TEST_CASE_NAME, TEST_NAME) \
	void TEST_CASE_NAME##_##TEST_NAME(bool * failed)

#define UNITTEST_EXPECT_EQ(expect, actual) \
    retval_testfunc = actual; \
    if ((typeof(actual))(ENOTSUP) != (typeof(actual))(retval_testfunc)) \
    { \
        fprintf( \
            stdout \
            , "LN: %04d EXPECT_EQ (%s, %s); \n" \
            , __LINE__ \
            , #expect \
            , #actual); \
        if ((typeof(actual))expect != (typeof(actual))retval_testfunc) \
        { \
            CHANGE_COLOR_PURPLE(); \
            fprintf( \
                stdout \
                , "LN: %04d %s(" REG_LD ") is not equal to the expect value: " REG_LD " \n" \
                , __LINE__ \
                , #actual \
                , retval_testfunc \
                , (int64_t)expect); \
            RECOVER_COLOR(); \
            *failed = true; \
            total_failed_count ++; \
        } \
    }

#define UNITTEST_EXPECT_NE(expect, actual) \
    retval_testfunc = actual; \
    if ((typeof(actual))(ENOTSUP) != (typeof(actual))(retval_testfunc)) \
    { \
        fprintf( \
            stdout \
            , "LN: %04d EXPECT_NQ (%s, %s); \n" \
            , __LINE__ \
            , #expect \
            , #actual ); \
        if ((typeof(actual))expect == (typeof(actual))retval_testfunc) \
        { \
            CHANGE_COLOR_PURPLE(); \
            fprintf( \
                stdout \
                , "LN: %04d %s(" REG_LD ") is equal to the expect value: " REG_LD " \n" \
                , __LINE__ \
                , #actual \
                , retval_testfunc \
                , (int64_t)expect); \
            RECOVER_COLOR(); \
            *failed = true; \
            total_failed_count ++; \
        } \
    }

#define UNITTEST_ADD_TESTCASE(TEST_CASE_NAME, TEST_NAME) \
    unittest_add_test_case(#TEST_CASE_NAME \
    , #TEST_NAME \
    , TEST_CASE_NAME##_##TEST_NAME)

void * unittest_add_test_case(
    const char * test_case_name
    , const char * test_name
    , void (* test_func)(bool *));

int32_t unittest_wait_child_proc_end(uint32_t timeout);

#endif // __CPLUS_UNITTEST__
#endif // __COMMON_H__