#include "cplus.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <errno.h>
#include <sys/wait.h>

#define UNUSED_PARAM(PARAM) (void)(PARAM)

static inline int32_t unittest_wait_child_proc_end(uint32_t timeout)
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