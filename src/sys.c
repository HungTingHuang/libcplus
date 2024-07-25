/******************************************************************
* @file: sys.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include "common.h"
#include "cplus_sys.h"

const char * cplus_sys_skip_file_path(const char * filepath)
{
    const char * target = NULL;
    if ((target = strrchr(filepath, '\\')))
    {
        return target + 1;
    }
    else
    {
        if ((target = strrchr(filepath, '/')))
        {
            return target + 1;
        }
        else
        {
            return filepath;
        }
    }
}

void cplus_sys_print_stack(void)
{
    void * callstack[128];
    int32_t i = 0, frames = backtrace(callstack, 128);
    char ** strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i)
    {
        printf("%s\n", strs[i]);
    }
    free(strs);
}

#ifdef __CPLUS_UNITTEST__
CPLUS_UNIT_TEST(cplus_sys_print_stack, functionity)
{
    cplus_sys_print_stack();
}

void unittest_sys(void)
{
    UNITTEST_ADD_TESTCASE(cplus_sys_print_stack, functionity);
}
#endif // __CPLUS_UNITTEST__