/******************************************************************
* @file: unittest.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include "cplus.h"
#include "common.h"

#define OBJ_TYPE (OBJ_NONE + HELPER + 0)
#define UNITTEST_NAME_MAX_SIZE 63

int64_t retval_testfunc = 0;
int32_t total_failed_count = 0;

struct test_case
{
    char test_case_name[UNITTEST_NAME_MAX_SIZE + 1];
    char test_name[UNITTEST_NAME_MAX_SIZE + 1];
    void (*test_case_func)(bool * failed);
    struct test_case * next;
};

static pid_t parents_id = 0;
static struct test_case * test_cases = NULL;

int32_t unittest_wait_child_proc_end(uint32_t timeout)
{
    while(0 < timeout)
    {
        if (0 < waitpid((pid_t)(-1), 0, WNOHANG) || 0 >= timeout)
        {
            return CPLUS_SUCCESS;
        }
        else
        {
            usleep(500 * 1000);
            timeout -= 500;
        }
    }

    errno = ETIMEDOUT;
    return CPLUS_FAIL;
}

void * unittest_add_test_case(
    const char * test_case_name
    , const char * test_name
    , void (* test_func)(bool *))
{
    struct test_case * cs = NULL;
    struct test_case * last_case = NULL;

    cs = (struct test_case *)malloc(sizeof(struct test_case));
    if (cs)
    {
        cplus_str_printf(cs->test_case_name, UNITTEST_NAME_MAX_SIZE, "%s", test_case_name);
        cplus_str_printf(cs->test_name, UNITTEST_NAME_MAX_SIZE, "%s", test_name);
        cs->test_case_func = test_func;
        cs->next = NULL;

        if (NULL == test_cases)
        {
            test_cases = cs;
        }
        else
        {
            for (last_case = test_cases
                ; (NULL != last_case) and (NULL != last_case->next)
                ; last_case = last_case->next) { }

            last_case->next = cs;
        }
    }
    return cs;
}

void unittest_run(void)
{
    struct test_case * next_case = NULL;

    fprintf(stdout, "unit test begin ... \n");
    while (NULL != test_cases)
    {
        next_case = test_cases->next;

        CHANGE_COLOR_YELLOW();
        fprintf(stdout, "%s: ", test_cases->test_case_name);
        CHANGE_COLOR_BLUE();
        fprintf(stdout, "%s\n", test_cases->test_name);
        RECOVER_COLOR();

        bool failed = false;
        test_cases->test_case_func(&failed);
        free(test_cases);

        test_cases = next_case;

        if (getpid() != parents_id)
        {
            fprintf(stdout, "LN: %04d CHILD PROCESS END!! result: ", __LINE__);

            /* clean all testcase */
            while (NULL != test_cases)
            {
                next_case = test_cases->next;
                free(test_cases);
                test_cases = next_case;
            }
        }
        else
        {
            fprintf(stdout, "test result: ");
        }

        if (true == failed)
        {
            CHANGE_COLOR_RED();
            fprintf(stdout, "FAIL");
        }
        else
        {
            CHANGE_COLOR_GREEN();
            fprintf(stdout, "PASS");
        }
        RECOVER_COLOR();

        if (getpid() != parents_id)
        {
            fprintf(stdout, ", failed: %d\n", total_failed_count);
            exit(0);
        }

        fprintf(stdout, "\n");
    }
    fprintf(stdout, "Test end!! All failed: %d\n", total_failed_count);
}

extern void unittest_atomic(void);
extern void unittest_memmgr(void);
extern void unittest_mempool(void);
extern void unittest_llist(void);
extern void unittest_sharedmem(void);
extern void unittest_rwlock(void);
extern void unittest_pevent(void);
extern void unittest_semaphore(void);
extern void unittest_task(void);
extern void unittest_mutex(void);
extern void unittest_taskpool(void);
extern void unittest_syslog(void);
extern void unittest_data(void);
extern void unittest_helper(void);
extern void unittest_sys(void);
extern void unittest_socket(void);
extern void unittest_ipc_server(void);
extern void unittest_file(void);

int32_t main(void)
{
    parents_id = getpid();

    unittest_atomic();
    unittest_memmgr();
    unittest_mempool();
    unittest_llist();
    unittest_sharedmem();
    unittest_rwlock();
    unittest_pevent();
    unittest_semaphore();
    unittest_task();
    unittest_mutex();
    unittest_taskpool();
    unittest_syslog();
    unittest_data();
    unittest_helper();
    unittest_sys();
    unittest_socket();
    unittest_ipc_server();
    unittest_file();

    unittest_run();
    return 0;
}