/******************************************************************
* @file: memmgr.c
*
* @author: Hunter Huang <bill.b750121@gmail.com>
******************************************************************/

#include <pthread.h>
#include <stdarg.h>
#include "common.h"
#include "cplus_memmgr.h"
#include "cplus_sys.h"

#define OBJ_TYPE (OBJ_NONE + CORE + 0)

#ifdef __CPLUS_MEM_MANAGER__
#define MAX_FILE_NAME 31
#define MAX_FUNC_NAME 63
#define BEGCHK 0x1A2A3A4A
#define ENDCHK 0x5B6B7B8B
#define CLRCHK 0xABABABAB

struct extra_info
{
	void * mem_ref;
	uint32_t mem_size;
	uint32_t line;
	struct extra_info * next;
	char file[MAX_FILE_NAME + 1];
	char function[MAX_FUNC_NAME + 1];
};

static struct extra_info * ptr_info_start = CPLUS_NULL;
static struct extra_info * ptr_info_last =  CPLUS_NULL;
static int32_t inused_mem_num = 0;
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
static const char meminfo_record_format[] = "(address) (size) (allocated by)\n %-10p %-10u  %s(%05u): %s\n";

static void printf_report_title(FILE * fd, bool * print)
{
	if (true == *print)
	{
		fprintf(fd, "======================================================================================\n");
		fprintf(fd, "MEMORY Report:\n");
		fprintf(fd, "--------------------------------------------------------------------------------------\n");
		*print = false;
	}
}
static void printf_report_summary(FILE * fd, bool * print, int32_t last_count)
{
	if (true == *print)
	{
		fprintf(fd, "--------------------------------------------------------------------------------------\n");
		fprintf(fd, "inuse count = %d\n", last_count);
		fprintf(fd, "======================================================================================\n");
		*print = false;
	}
}
static int32_t add_mem_info(
	void * mem_ref
	, uint32_t size
	, const char * file
	, const char * function
	, uint32_t line)
{
	struct extra_info * ex = CPLUS_NULL;
	CHECK_NOT_NULL(file, CPLUS_FAIL);
	CHECK_NOT_NULL(function, CPLUS_FAIL);

	if ((ex = (struct extra_info *)malloc(sizeof(struct extra_info))))
	{
		ex->mem_ref = mem_ref;
		ex->mem_size = size;
		snprintf(ex->file, MAX_FILE_NAME, "%s", cplus_sys_skip_file_path(file));
		snprintf(ex->function, MAX_FUNC_NAME, "%s", function);
		ex->line = line;
		ex->next = CPLUS_NULL;

		pthread_rwlock_wrlock(&rwlock);

		if (CPLUS_NULL == ptr_info_start)
		{
			ptr_info_start = ex;
			ptr_info_last = ptr_info_start;
		}
		else
		{
			ptr_info_last->next = ex;
			ptr_info_last = ptr_info_last->next;
		}
		inused_mem_num ++;
		pthread_rwlock_unlock(&rwlock);

		return CPLUS_SUCCESS;
	}

	errno = ENOMEM;
	return ENOMEM;
}
static void * find_mem_info(void * mem_addr)
{
	struct extra_info * ptr_temp = ptr_info_start;
	CHECK_NOT_NULL(mem_addr, CPLUS_NULL);

	pthread_rwlock_rdlock(&rwlock);
	while (ptr_temp)
	{
		if (ptr_temp->mem_ref == mem_addr)
		{
			pthread_rwlock_unlock(&rwlock);
			return ptr_temp;
		}
		ptr_temp = ptr_temp->next;
	}
	pthread_rwlock_unlock(&rwlock);

	return CPLUS_NULL;
}
static int32_t erase_mem_info(void * mem_addr)
{
	struct extra_info * ptr_temp = ptr_info_start, *ptr_temp_prev = CPLUS_NULL;
	CHECK_NOT_NULL(mem_addr, CPLUS_FAIL);

	pthread_rwlock_wrlock(&rwlock);

	while (ptr_temp)
	{
		if (ptr_temp->mem_ref == mem_addr)
		{
			if (ptr_temp == ptr_info_start)
			{
				ptr_info_start = ptr_temp->next;
			}
			else if (ptr_temp == ptr_info_last)
			{
				ptr_info_last = ptr_temp_prev;
				ptr_info_last->next = CPLUS_NULL;
			}
			else
			{
				ptr_temp_prev->next = ptr_temp->next;
			}

			free(ptr_temp);
			ptr_temp = CPLUS_NULL;

			inused_mem_num --;
			break;
		}

		ptr_temp_prev = ptr_temp;
		ptr_temp = ptr_temp->next;
	}

	pthread_rwlock_unlock(&rwlock);

	return CPLUS_SUCCESS;
}
static int32_t check_size(void * mem_addr, uint32_t size)
{
	struct extra_info * target = CPLUS_NULL;

	if ((target = (struct extra_info *)find_mem_info(mem_addr)))
	{
		if (target->mem_size < size)
		{
			fprintf(
				stdout
				, "[OVERFLOW] original memory size = %d, writen size: %d, overtake: %d\n"
				, target->mem_size
				, size
				, (size - target->mem_size));
			return CPLUS_FAIL;
		}
	}
	return CPLUS_SUCCESS;
}
static int32_t check_boundary(void * entire)
{
	uint32_t * begin_tag = ((uint32_t *)entire), end_check = ENDCHK;
	void * target = (void *)(((uint8_t *)entire) + sizeof(uint32_t));

	struct extra_info * info = (struct extra_info *)find_mem_info(target);
	if (info)
	{
		if (CLRCHK == (* begin_tag))
		{
			fprintf(stdout, "[DOUBLE FREE]\n");
			fprintf(
				stdout
				, meminfo_record_format
				, info->mem_ref
				, info->mem_size
				, info->file
				, info->line
				, info->function);
			return CPLUS_FAIL;
		}
		else if (BEGCHK != (* begin_tag))
		{
			fprintf(stdout, "[INVAILID POINTER]\n");
			fprintf(
				stdout
				, meminfo_record_format
				, info->mem_ref
				, info->mem_size
				, info->file
				, info->line
				, info->function);
			return CPLUS_FAIL;
		}
		else if (memcmp(
			(void *)(((uint8_t *)target) + (info->mem_size))
			, &(end_check)
			, sizeof(uint32_t)))
		{
			fprintf(stdout, "[OVERFLOW]\n");
			fprintf(
				stdout
				, meminfo_record_format
				, info->mem_ref
				, info->mem_size
				, info->file
				, info->line
				, info->function);
			return CPLUS_FAIL;
		}
	}
	return CPLUS_SUCCESS;
}

void * cplus_mgr_malloc(
	uint32_t size
	, const char * file
	, const char * function
	, uint32_t line)
{
	void * mem = CPLUS_NULL, * target = CPLUS_NULL;
	uint32_t * begin_tag = CPLUS_NULL, end_check = ENDCHK;

	if (size)
	{
		mem = (void *)malloc(size + (2 * sizeof(uint32_t)));
		if (mem)
		{
			begin_tag = ((uint32_t *)mem);
			(* begin_tag) = BEGCHK;
			target = (void *)(((uint8_t *)mem) + sizeof(uint32_t));
			memcpy((void *)(((uint8_t *)target) + size), &(end_check), sizeof(uint32_t));
			add_mem_info(target, size, file, function, line);
		}
	}
	return target;
}
void * cplus_mgr_realloc(
	void * ptr
	, uint32_t size
	, const char * file
	, const char * function
	, uint32_t line)
{
	void * mem = CPLUS_NULL, * target = CPLUS_NULL, * entire = CPLUS_NULL;
	uint32_t new_size = 0, * begin_tag = CPLUS_NULL, end_check = ENDCHK;
	CHECK_NOT_NULL(ptr, CPLUS_NULL);

	entire = (void *)(((uint8_t *)ptr) - sizeof(uint32_t));
	assert(CPLUS_SUCCESS == check_boundary(entire));
	erase_mem_info(ptr);

	new_size = (0 == size)? 0: (size + (2 * sizeof(uint32_t)));
	if (CPLUS_NULL == (mem = realloc(entire, new_size)))
	{
		if (0 != size)
		{
			free(entire);
		}
		errno = ENOMEM;
		return CPLUS_NULL;
	}

	begin_tag = ((uint32_t *)mem);
	(* begin_tag) = BEGCHK;
	target = (void *)(((uint8_t *)mem) + sizeof(uint32_t));
	memcpy((void *)(((uint8_t *)target) + size), &(end_check), sizeof(uint32_t));

	add_mem_info(target, size, file, function, line);
	return target;
}
#else
void * cplus_mgr_malloc(uint32_t size)
{
	return (void *)malloc(size);
}
void * cplus_mgr_realloc(void * ptr, uint32_t size)
{
	void * mem = CPLUS_NULL;
	CHECK_NOT_NULL(ptr, CPLUS_NULL);

	if (CPLUS_NULL == (mem = realloc(ptr, size)))
	{
		if (0 != size)
		{
			free(ptr);
		}
		errno = ENOMEM;
	}

	return mem;
}
#endif // __CPLUS_MEM_MANAGER__

int32_t cplus_mgr_check_size(void * ptr, uint32_t size)
{
#ifdef __CPLUS_MEM_MANAGER__
	CHECK_NOT_NULL(ptr, CPLUS_FAIL);
	CHECK_GT_ZERO(size, CPLUS_FAIL);
	return check_size(ptr, size);
#else
	UNUSED_PARAM(ptr);
	UNUSED_PARAM(size);
	return CPLUS_SUCCESS;
#endif
}

int32_t cplus_mgr_check_boundary(void * ptr)
{
#ifdef __CPLUS_MEM_MANAGER__
	void * entire = CPLUS_NULL;
	CHECK_NOT_NULL(ptr, CPLUS_FAIL);
	entire = (void *)(((uint8_t *)ptr) - sizeof(uint32_t));
	return check_boundary(entire);
#else
	UNUSED_PARAM(ptr);
	return CPLUS_SUCCESS;
#endif
}

int32_t cplus_mgr_report_in_file(void * stream)
{
	int32_t used_num = 0;
#ifdef __CPLUS_MEM_MANAGER__
	bool print_title = true, print_summary;
	FILE * output = stdout;

	if (stream)
	{
		output = (FILE *)stream;
	}

	pthread_rwlock_rdlock(&rwlock);

	struct extra_info * ptr_temp = ptr_info_start;
	while (ptr_temp)
	{
		printf_report_title(output, &print_title);

		fprintf(
			output
			, meminfo_record_format
			, ptr_temp->mem_ref
			, ptr_temp->mem_size
			, ptr_temp->file
			, ptr_temp->line
			, ptr_temp->function);

		used_num ++;
		ptr_temp = ptr_temp->next;
	}

	pthread_rwlock_unlock(&rwlock);

	if (0 != used_num)
	{
		print_summary = true;
		printf_report_summary(output, &print_summary, used_num);
	}
#else
	UNUSED_PARAM(stream);
	errno = ENOTSUP;
#endif
	return used_num;
}

int32_t cplus_mgr_report(void)
{
	return cplus_mgr_report_in_file(CPLUS_NULL);
}

int32_t cplus_mgr_free(void * ptr)
{
	CHECK_NOT_NULL(ptr, EINVAL);
#ifdef __CPLUS_MEM_MANAGER__
	uint32_t * begin_tag = CPLUS_NULL;
	void * entire = (void *)(((uint8_t *)ptr) - sizeof(uint32_t));
	assert(CPLUS_SUCCESS == check_boundary(entire));

	begin_tag = ((uint32_t *)entire);
	(* begin_tag) = CLRCHK;

	erase_mem_info(ptr);
	free(entire);
#else
	free(ptr);
#endif
	return CPLUS_SUCCESS;
}

void * cplus_mem_cpy_ex(
	void * dest
	, uint32_t destsz
	, void * src
	, uint32_t count)
{
	uint8_t * dp = (uint8_t *)(dest);
	const uint8_t * sp = (const uint8_t *)(src);

	CHECK_NOT_NULL(dp, CPLUS_NULL);
	CHECK_NOT_NULL(sp, CPLUS_NULL);
	CHECK_IF(0 == destsz, CPLUS_NULL);
	CHECK_IF(destsz > RSIZE_MAX_MEM, CPLUS_NULL);
	CHECK_IF(0 == count, CPLUS_NULL);
	CHECK_IF(count > destsz, CPLUS_NULL);

#ifdef __CPLUS_MEM_MANAGER__
	assert(CPLUS_SUCCESS == check_size(dest, count));
#endif

	if (((dp > sp) AND (dp < (sp + count)))
		OR ((sp > dp) AND (sp < (dp + destsz))))
	{
		return memmove(dp, sp, count);
	}

	return memcpy(dp, sp, count);
}

void * cplus_mem_set(
	void * dest
	, uint8_t value
	, uint32_t count)
{
	uint8_t * dp = (uint8_t *)(dest);

	CHECK_NOT_NULL(dp, CPLUS_NULL);
	CHECK_IF(count > RSIZE_MAX_MEM, CPLUS_NULL);
	CHECK_IF(0 == count, CPLUS_NULL);

#ifdef __CPLUS_MEM_MANAGER__
	assert(CPLUS_SUCCESS == check_size(dest, count));
#endif
	return memset(dp, value, count);
}

void * cplus_mem_cpy(void * dest, void * src, uint32_t count)
{
	uint8_t * dp = (uint8_t *)(dest);
	const uint8_t * sp = (const uint8_t *)(src);

	CHECK_NOT_NULL(dp, CPLUS_NULL);
	CHECK_NOT_NULL(sp, CPLUS_NULL);
	CHECK_IF(0 == count, CPLUS_NULL);

#ifdef __CPLUS_MEM_MANAGER__
	assert(CPLUS_SUCCESS == check_size(dest, count));
#endif
	return memcpy(dp, sp, count);
}

int32_t cplus_str_printf(
	void * str
	, uint32_t size
	, const char * format
	, ...)
{
	int32_t res = 0;
	va_list args;

#ifdef __CPLUS_MEM_MANAGER__
	assert(CPLUS_SUCCESS == check_size(str, size));
#endif

	va_start(args, format);
	res = vsnprintf((char *)(str), size, format, args);
	va_end(args);

	return res;
}

#ifdef __CPLUS_UNITTEST__
#define MAX_TEST_SIZE 1024
int32_t *a = CPLUS_NULL, *b = CPLUS_NULL, *c = CPLUS_NULL, *d = CPLUS_NULL, *e = CPLUS_NULL, *f = CPLUS_NULL;
int64_t *g = CPLUS_NULL, *h = CPLUS_NULL, *i = CPLUS_NULL, *j = CPLUS_NULL, *k = CPLUS_NULL, *l = CPLUS_NULL;
int32_t *ry[MAX_TEST_SIZE];
int64_t *ry2[MAX_TEST_SIZE];

static struct test1
{
	int32_t A;
	int32_t B;
	int32_t C;
	int32_t D;
	int32_t E;
} t1 = { .A = 0x0123, .B = 0x1234, .C = 0x2345, .D = 0x3456, .E = 0x4567 };

CPLUS_UNIT_TEST(cplus_mgr_malloc, functionity)
{
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(a));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(b));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(c));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(d));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(e));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(f));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mgr_malloc, stress)
{
	for (int32_t i = 0; i < MAX_TEST_SIZE; i++)
	{
		UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (ry[i] = (int32_t *)cplus_malloc(sizeof(int32_t))));
	}
	UNITTEST_EXPECT_EQ(MAX_TEST_SIZE, cplus_mgr_report());

	for (int32_t i = 0; i < MAX_TEST_SIZE; i++)
	{
		UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (ry2[i] = (int64_t *)cplus_realloc(ry[i], sizeof(int64_t))));
	}
	UNITTEST_EXPECT_EQ(MAX_TEST_SIZE, cplus_mgr_report());

	for (int32_t i = 0; i< MAX_TEST_SIZE; i++)
	{
		UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(ry2[i]));
	}
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mgr_realloc, functionity)
{
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (g = (int64_t *)cplus_realloc(a, sizeof(int64_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (h = (int64_t *)cplus_realloc(b, sizeof(int64_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (i = (int64_t *)cplus_realloc(c, sizeof(int64_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (j = (int64_t *)cplus_realloc(d, sizeof(int64_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (k = (int64_t *)cplus_realloc(e, sizeof(int64_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (l = (int64_t *)cplus_realloc(f, sizeof(int64_t))));
	UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(g));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(h));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(i));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(j));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(k));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(l));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mgr_realloc, bad_parameter)
{
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (g = (int64_t *)cplus_realloc(a, 0)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (h = (int64_t *)cplus_realloc(b, 0)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (i = (int64_t *)cplus_realloc(c, 0)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (j = (int64_t *)cplus_realloc(d, 0)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (k = (int64_t *)cplus_realloc(e, 0)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (l = (int64_t *)cplus_realloc(f, 0)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (a = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (b = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (c = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (d = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (e = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (f = (int32_t *)cplus_malloc(sizeof(int32_t))));
	UNITTEST_EXPECT_EQ(6, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (g = (int64_t *)cplus_realloc(a
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (h = (int64_t *)cplus_realloc(b
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (i = (int64_t *)cplus_realloc(c
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (j = (int64_t *)cplus_realloc(d
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (k = (int64_t *)cplus_realloc(e
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL == (l = (int64_t *)cplus_realloc(f
		, 0xffffffff * 10000000 * 10000000 * 10000000 * 10000000 * 10000000 * 10000000)));
	UNITTEST_EXPECT_EQ(ENOMEM, errno);
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(CPLUS_MEM_CLEAN, functionity)
{
	UNITTEST_EXPECT_EQ(0x0123, t1.A);
	UNITTEST_EXPECT_EQ(0x1234, t1.B);
	UNITTEST_EXPECT_EQ(0x2345, t1.C);
	UNITTEST_EXPECT_EQ(0x3456, t1.D);
	UNITTEST_EXPECT_EQ(0x4567, t1.E);
	CPLUS_MEM_CLEAN(&(t1));
	UNITTEST_EXPECT_EQ(0, t1.A);
	UNITTEST_EXPECT_EQ(0, t1.B);
	UNITTEST_EXPECT_EQ(0, t1.C);
	UNITTEST_EXPECT_EQ(0, t1.D);
	UNITTEST_EXPECT_EQ(0, t1.E);
}

#ifdef __CPLUS_MEM_MANAGER__
CPLUS_UNIT_TEST(cplus_mgr_malloc, buffer_overrun)
{
	uint8_t * str = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
	str[10] = '\0';
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mgr_check_boundary(str));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mgr_realloc, buffer_overrun)
{
	uint8_t * str = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
	str[9] = '\0';
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_realloc(str, 15)));
	UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
	str[15] = '\0';
	UNITTEST_EXPECT_EQ(CPLUS_FAIL, cplus_mgr_check_boundary(str));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mgr_malloc, double_free)
{
	uint8_t * str = CPLUS_NULL, * temp = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	temp = str;
	UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(temp));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mgr_realloc, double_free)
{
	uint8_t * str = CPLUS_NULL, * temp = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(10)));
	UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_realloc(str, 15)));
	UNITTEST_EXPECT_EQ(1, cplus_mgr_report());
	temp = str;
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(str));
	UNITTEST_EXPECT_EQ(CPLUS_SUCCESS, cplus_free(temp));
	UNITTEST_EXPECT_EQ(0, cplus_mgr_report());
}

CPLUS_UNIT_TEST(cplus_mem_cpy, overflow)
{
	uint8_t * str = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	UNITTEST_EXPECT_EQ(true, (str == cplus_mem_cpy(str, (void *)("012345"), strlen("012345"))));
}

CPLUS_UNIT_TEST(cplus_mem_cpy_ex, overflow)
{
	uint8_t * str = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	UNITTEST_EXPECT_EQ(true, (str == cplus_mem_cpy_ex(str, strlen("012345"), (void *)("012345"), strlen("012345"))));
}

CPLUS_UNIT_TEST(cplus_mem_set, overflow)
{
	uint8_t * str = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	UNITTEST_EXPECT_EQ(true, (str == cplus_mem_set(str, 0x00, 6)));
}

CPLUS_UNIT_TEST(cplus_str_printf, overflow)
{
	uint8_t * str = CPLUS_NULL;
	UNITTEST_EXPECT_EQ(true, CPLUS_NULL != (str = (uint8_t *)cplus_malloc(5)));
	UNITTEST_EXPECT_EQ(true, (strlen("012345") == cplus_str_printf(str, strlen("012345"), "%s", strlen("012345"))));
}
#endif

void unittest_memmgr(void)
{
	UNITTEST_ADD_TESTCASE(cplus_mgr_malloc, functionity);
	UNITTEST_ADD_TESTCASE(cplus_mgr_malloc, stress);
	UNITTEST_ADD_TESTCASE(cplus_mgr_realloc, functionity);
	UNITTEST_ADD_TESTCASE(cplus_mgr_realloc, bad_parameter);
	UNITTEST_ADD_TESTCASE(CPLUS_MEM_CLEAN, functionity);
#ifdef __CPLUS_MEM_MANAGER__
	// UNITTEST_ADD_TESTCASE(cplus_mgr_malloc, buffer_overrun);
	// UNITTEST_ADD_TESTCASE(cplus_mgr_realloc, buffer_overrun);
	// UNITTEST_ADD_TESTCASE(cplus_mgr_malloc, double_free);
	// UNITTEST_ADD_TESTCASE(cplus_mgr_realloc, double_free);
	// UNITTEST_ADD_TESTCASE(cplus_mem_cpy, overflow);
	// UNITTEST_ADD_TESTCASE(cplus_mem_cpy_ex, overflow);
	// UNITTEST_ADD_TESTCASE(cplus_mem_set, overflow);
	// UNITTEST_ADD_TESTCASE(cplus_str_printf, overflow);
#endif
}
#endif // __CPLUS_UNITTEST__