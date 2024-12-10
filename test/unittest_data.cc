#include "internal.h"

TEST(cplus_data_new, CPLUS_DATA_TYPE_BOOL)
{
	bool result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_bool(true)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool"), "test_bool"));
	EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool_bool_bool_bool"), "test_bool_bool_bool_bool"));
	EXPECT_EQ(0, strcmp("test_bool_bool_bool_bool", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(bool), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ(CPLUS_TRUE, result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_bool(data, false));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_get_bool(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT8)
{
	int8_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int8(0xab)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int8"), "test_int8"));
	EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT8, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int8_t)(0xab), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int8(data, 0xba));
	EXPECT_EQ((int8_t)(0xba), cplus_data_get_int8(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT16)
{
	int16_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int16(0xabcd)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int16"), "test_int16"));
	EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT16, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int16_t)(0xabcd), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int16(data, 0xdcba));
	EXPECT_EQ((int16_t)(0xdcba), cplus_data_get_int16(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT32)
{
	int32_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32(123456)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int32"), "test_int32"));
	EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT32, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int32_t)(123456), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int32(data, -123456));
	EXPECT_EQ((int32_t)(-123456), cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT64)
{
	int64_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int64(0x0123abcddcba3210)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int64"), "test_int64"));
	EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT64, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int64_t)(0x0123abcddcba3210), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int64(data, 0x7FFFFFFFFFFFFFFF));
	EXPECT_EQ((int64_t)(0x7FFFFFFFFFFFFFFF), cplus_data_get_int64(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT8)
{
	uint8_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint8(0xab)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint8"), "test_uint8"));
	EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint8_t)(0xab), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint8(data, 0xba));
	EXPECT_EQ((uint8_t)(0xba), cplus_data_get_uint8(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT16)
{
	uint16_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint16(0xabcd)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint16"), "test_uint16"));
	EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint16_t)(0xabcd), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint16(data, 0xdcba));
	EXPECT_EQ((uint16_t)(0xdcba), cplus_data_get_uint16(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT32)
{
	uint32_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint32(123456)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint32"), "test_uint32"));
	EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint32_t)(123456), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint32(data, 6543210));
	EXPECT_EQ((uint32_t)(6543210), cplus_data_get_uint32(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT64)
{
	uint64_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint64(0x0123abcddcba3210)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint64"), "test_uint64"));
	EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint64_t)(0x0123abcddcba3210), result);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint64(data, 0xdcba32100123abcd));
	EXPECT_EQ((uint64_t)(0xdcba32100123abcd), cplus_data_get_uint64(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_FLOAT)
{
	float result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_float(3.1415926535)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_float"), "test_float"));
	EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(float), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_FLOAT, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(float), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - result) < 0.0000001f);
	// EXPECT_EQ(0, result - 3.14159);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_float(data, 2.7182818284));
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_float(data)) < 0.01f);
	// EXPECT_EQ(0, cplus_data_get_float(data) - 2.71828);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_DOUBLE)
{
	double result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_double(3.1415926535)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_double"), "test_double"));
	EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(double), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_DOUBLE, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(double), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - result) < 0.0000001f);
	// EXPECT_EQ(0, result - 3.14159);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_double(data, 2.7182818284));
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_double(data)) < 0.0000001f);
	// EXPECT_EQ(0, cplus_data_get_double(data) - 2.71828);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_POINTER)
{
	int32_t v1 = 123456, v2 = 654321;
	void * result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_pointer(&v1)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_pointer"), "test_pointer"));
	EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(intptr_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_POINTER, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int32_t)(123456), *((int32_t *)result));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_pointer(data, &v2));
	EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int32_t)(654321), *((int32_t *)result));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_pointer(data, &v1));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (result = cplus_data_get_pointer(data))));
	EXPECT_EQ((int32_t)(123456), *((int32_t *)result));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_STRING)
{
	char bufs[32];
	uint32_t len = sizeof(bufs);
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_string(strlen("Hello World"), (char *)("Hello World"))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_string"), (char *)("test_string")));
	EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
	EXPECT_EQ(strlen("Hello World"), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_STRING, cplus_data_get_type(data));
	EXPECT_EQ(strlen("Hello World"), cplus_data_get_value(data, &len, bufs));
	EXPECT_EQ(0, strcmp("Hello World", bufs));
	EXPECT_EQ(0, cplus_data_set_string(data, strlen("AaBbCcDd"), (char *)("AaBbCcDd")));
	EXPECT_EQ(strlen("AaBbCcDd"), cplus_data_get_data_size(data));
	EXPECT_EQ(strlen("AaBbCcDd"), strlen(cplus_data_get_string(data)));
	EXPECT_EQ(0, strcmp("AaBbCcDd", cplus_data_get_string(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_string_code(data, 0xabcd));
	EXPECT_EQ(0xabcd, cplus_data_get_string_code(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_BYTE_ARRAY)
{
	uint8_t bufs[32] = {0}, get_value[32] = {0}, test_value[64] = {0};
	uint32_t get_len = 32;
	cplus_data data = CPLUS_NULL;
	for (uint32_t i = 0; i < sizeof(bufs); i ++)
	{
		bufs[i] = 0xAB;
	}
	test_value[0] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_byte_array(sizeof(bufs), bufs)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_byte_array"), "test_byte_array"));
	EXPECT_EQ(0, strcmp("test_byte_array", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(bufs), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_BYTE_ARRAY, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(bufs), cplus_data_get_value(data, &get_len, get_value));
	EXPECT_EQ(0, memcmp(bufs, get_value, sizeof(bufs)));
	EXPECT_EQ(0, cplus_data_set_byte_array(data, 1, test_value));
	EXPECT_EQ(1, cplus_data_get_data_size(data));
	EXPECT_EQ(0, memcmp(test_value, cplus_data_get_byte_array(data), 1));
	EXPECT_EQ(0, cplus_data_set_byte_array(data, 1, test_value));
	EXPECT_EQ(1, cplus_data_get_data_size(data));
	EXPECT_EQ(0, memcmp(test_value, cplus_data_get_byte_array(data), 1));
	for (uint32_t i = 0; i < sizeof(test_value); i ++)
	{
		test_value[i] = 0xBC;
	}
	EXPECT_EQ(0, cplus_data_set_byte_array(data, sizeof(test_value), test_value));
	EXPECT_EQ(sizeof(test_value), cplus_data_get_data_size(data));
	EXPECT_EQ(0, memcmp(test_value, cplus_data_get_byte_array(data), sizeof(test_value)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_BOOL)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_bool(true)));
	EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, 1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 1 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 1 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 1 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 1 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 1 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 1 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 1 == db);
	EXPECT_EQ(strlen("true"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "true"));
	EXPECT_EQ(1, cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0x01;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, 1)));
	EXPECT_EQ(0, cplus_data_set_bool(data, false));
	EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, false == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, 0 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 0 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 0 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 0 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 0 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 0 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 0 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 0 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 0 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 0 == db);
	EXPECT_EQ(strlen("false"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "false"));
	EXPECT_EQ(1, cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0x00;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, 1)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT8)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int8(127)));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT8, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, 127 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 127 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 127 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 127 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 127 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 127 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 127 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 127 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 127 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 127 == db);
	EXPECT_EQ(strlen("127"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "127"));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int8_t))));
	EXPECT_EQ(0, cplus_data_set_int8(data, -1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, -1 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, (uint64_t)(-1) == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, -1 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, -1 == db);
	EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "-1"));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int8_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT16)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int16(32767)));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT16, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 32767 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 32767 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 32767 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 32767 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 32767 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 32767 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 32767 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 32767 == db);
	EXPECT_EQ(strlen("32767"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "32767"));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int16_t))));
	EXPECT_EQ(0, cplus_data_set_int16(data, -1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, -1 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, (uint64_t)(-1) == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, -1 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, -1 == db);
	EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "-1"));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int16_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT32)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32(2147483647)));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT32, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == db);
	EXPECT_EQ(strlen("2147483647"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "2147483647"));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int32_t))));
	EXPECT_EQ(0, cplus_data_set_int32(data, -1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, -1 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, (uint64_t)(-1) == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, -1 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, -1 == db);
	EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "-1"));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int32_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_INT64)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int64(0x7fffffffffffffff)));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT64, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == db);
	EXPECT_EQ(strlen("9223372036854775807"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "9223372036854775807"));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0xff;
	test_value[4] = 0xff;
	test_value[5] = 0xff;
	test_value[6] = 0xff;
	test_value[7] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int64_t))));
	EXPECT_EQ(0, cplus_data_set_int64(data, -1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, -1 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, (uint64_t)(-1) == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, -1 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, -1 == db);
	EXPECT_EQ(strlen("-1"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "-1"));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0xff;
	test_value[4] = 0xff;
	test_value[5] = 0xff;
	test_value[6] = 0xff;
	test_value[7] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(int64_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT8)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint8(127)));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, 127 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 127 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 127 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 127 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 127 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 127 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 127 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 127 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 127 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 127 == db);
	EXPECT_EQ(strlen("127"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "127"));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint8_t))));
	EXPECT_EQ(0, cplus_data_set_uint8(data, 255));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 255 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 255 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 255 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 255 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 255 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 255 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 255 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 255 == db);
	EXPECT_EQ(strlen("255"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "255"));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint8_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT16)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint16(32767)));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, 32767 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 32767 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 32767 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 32767 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 32767 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 32767 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 32767 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 32767 == db);
	EXPECT_EQ(strlen("32767"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "32767"));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint16_t))));
	EXPECT_EQ(0, cplus_data_set_uint16(data, 65535));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 65535 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 65535 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 65535 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 65535 == db);
	EXPECT_EQ(strlen("65535"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "65535"));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint16_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT32)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint32(2147483647)));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 2147483647 == db);
	EXPECT_EQ(strlen("2147483647"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "2147483647"));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint32_t))));
	EXPECT_EQ(0, cplus_data_set_uint32(data, 4294967295));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == db);
	EXPECT_EQ(strlen("4294967295"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "4294967295"));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint32_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_get_as, CPLUS_DATA_TYPE_UINT64)
{
	cplus_data data = CPLUS_NULL;
	bool b; float f; double db; char str[64]; uint32_t len = sizeof(str);
	int8_t i8; int16_t i16; int32_t i32; int64_t i64;
	uint8_t u8; uint16_t u16; uint32_t u32; uint64_t u64;
	uint8_t bufs[32] = {0}, test_value[32] = {0}; uint32_t bufs_len = sizeof(bufs);

	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint64(0x7fffffffffffffff)));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 9223372036854775807 == db);
	EXPECT_EQ(strlen("9223372036854775807"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "9223372036854775807"));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0xff;
	test_value[4] = 0xff;
	test_value[5] = 0xff;
	test_value[6] = 0xff;
	test_value[7] = 0x7f;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint64_t))));
	EXPECT_EQ(0, cplus_data_set_uint64(data, 0xffffffffffffffff));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_bool(data, &b));
	EXPECT_EQ(CPLUS_TRUE, true == b);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int8(data, &i8));
	EXPECT_EQ(CPLUS_TRUE, -1 == i8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int16(data, &i16));
	EXPECT_EQ(CPLUS_TRUE, -1 == i16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int32(data, &i32));
	EXPECT_EQ(CPLUS_TRUE, -1 == i32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_int64(data, &i64));
	EXPECT_EQ(CPLUS_TRUE, -1 == i64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint8(data, &u8));
	EXPECT_EQ(CPLUS_TRUE, 255 == u8);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint16(data, &u16));
	EXPECT_EQ(CPLUS_TRUE, 65535 == u16);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint32(data, &u32));
	EXPECT_EQ(CPLUS_TRUE, 4294967295 == u32);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_uint64(data, &u64));
	EXPECT_EQ(CPLUS_TRUE, 0xffffffffffffffff == u64);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_float(data, &f));
	EXPECT_EQ(CPLUS_TRUE, 0xffffffffffffffff == f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_get_as_double(data, &db));
	EXPECT_EQ(CPLUS_TRUE, 0xffffffffffffffff == db);
	EXPECT_EQ(strlen("18446744073709551615"), cplus_data_get_as_string(data, len, str));
	EXPECT_EQ(CPLUS_TRUE, 0 == strcmp(str, "18446744073709551615"));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_as_byte_array(data, bufs_len, bufs));
	test_value[0] = 0xff;
	test_value[1] = 0xff;
	test_value[2] = 0xff;
	test_value[3] = 0xff;
	test_value[4] = 0xff;
	test_value[5] = 0xff;
	test_value[6] = 0xff;
	test_value[7] = 0xff;
	EXPECT_EQ(CPLUS_TRUE, (0 == memcmp(bufs, test_value, sizeof(uint64_t))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_set_as, CPLUS_DATA_TYPE_STRING)
{
	cplus_data data = CPLUS_NULL;
	char str[64]; uint32_t len = sizeof(str);
	/* char cmp[64]; uint32_t cmp_len = 0, u32 = 5566; */
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_string(strlen("123"), (char *)("123"))));
	EXPECT_EQ(strlen("123"), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_STRING, cplus_data_get_type(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_bool(data, true));
	EXPECT_EQ(strlen("true"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("true", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint8(data, 0xff));
	EXPECT_EQ(strlen("255"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("255", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint16(data, 0xffff));
	EXPECT_EQ(strlen("65535"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("65535", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint32(data, 0xffffffff));
	EXPECT_EQ(strlen("4294967295"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("4294967295", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_uint64(data, 0xffffffffffffffff));
	EXPECT_EQ(strlen("18446744073709551615"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("18446744073709551615", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_bool(data, false));
	EXPECT_EQ(strlen("false"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("false", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int8(data, 0xff));
	EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("-1", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int16(data, 0xffff));
	EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("-1", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int32(data, 0xffffffff));
	EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("-1", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_int64(data, 0xffffffffffffffff));
	EXPECT_EQ(strlen("-1"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("-1", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_float(data, 3.141593));
	EXPECT_EQ(strlen("3.141593"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("3.141593", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_double(data, 3.141593));
	EXPECT_EQ(strlen("3.141593"), cplus_data_get_value(data, &len, str));
	/*
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_pointer(data, &u32));
	EXPECT_EQ(CPLUS_TRUE,  0 < (cmp_len = snprintf(cmp, sizeof(cmp), "%p", &u32)));
	EXPECT_EQ(cmp_len, cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp(cmp, str));
	*/
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_as_string(data,
	strlen("012345678901234567890123456789012345678901234567890123456789123"), (char *)("012345678901234567890123456789012345678901234567890123456789123")));
	EXPECT_EQ(strlen("012345678901234567890123456789012345678901234567890123456789123"), cplus_data_get_value(data, &len, str));
	EXPECT_EQ(0, strcmp("012345678901234567890123456789012345678901234567890123456789123", str));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_is_valid, functionity)
{
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new(CPLUS_DATA_TYPE_NULL, CPLUS_NULL, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_bool(true)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int8(0xab)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int16(0xabcd)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32(123456)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int64(0x0123abcddcba3210)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint8(0xab)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint16(0xabcd)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint32(123456)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint64(0x0123abcddcba3210)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_float(3.1415926535)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_double(3.1415926535)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_string(strlen("Hello World"), (char *)("Hello World"))));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_NULL, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_BOOL, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_bool(data, true));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_INT8, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int8(data, 0xab));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_INT16, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int16(data, 0xabcd));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_INT32, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int32(data, 123456));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_INT64, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_int64(data, 0x0123abcddcba3210));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_UINT8, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint8(data, 0xab));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_UINT16, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint16(data, 0xabcd));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_UINT32, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint32(data, 123456));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_UINT64, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_uint64(data, 0x0123abcddcba3210));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_FLOAT, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_float(data, 3.1415926535));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_DOUBLE, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_double(data, 3.1415926535));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_POINTER, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_ex(
		CPLUS_DATA_TYPE_STRING, CPLUS_NULL, CPLUS_NULL, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_string(data, strlen("Hello World"), (char *)("Hello World")));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_is_valid(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_reverse, functionity)
{
	int16_t i16 = 0x0123;
	int32_t i32 = 0x01234567;
	int64_t i64 = 0x0123456789abcdef;
	uint16_t u16 = 0x0123;
	uint32_t u32 = 0x01234567;
	uint64_t u64 = 0x0123456789abcdef;

	EXPECT_EQ(0x0123, i16);
	EXPECT_EQ(0x0123, u16);
	EXPECT_EQ(0x2301, CPLUS_DATA_SWAP(i16));
	EXPECT_EQ(0x2301, CPLUS_DATA_SWAP(u16));
	EXPECT_EQ(0x0123, i16);
	EXPECT_EQ(0x0123, u16);
	EXPECT_EQ(0x01234567, i32);
	EXPECT_EQ(0x01234567, u32);
	EXPECT_EQ(0x67452301, CPLUS_DATA_SWAP(i32));
	EXPECT_EQ(0x67452301, CPLUS_DATA_SWAP(u32));
	EXPECT_EQ(0x01234567, i32);
	EXPECT_EQ(0x01234567, u32);
	EXPECT_EQ(0x0123456789abcdef, i64);
	EXPECT_EQ(0x0123456789abcdef, u64);
	EXPECT_EQ(0xefcdab8967452301, CPLUS_DATA_SWAP(i64));
	EXPECT_EQ(0xefcdab8967452301, CPLUS_DATA_SWAP(u64));
	EXPECT_EQ(0x0123456789abcdef, i64);
	EXPECT_EQ(0x0123456789abcdef, u64);
	EXPECT_EQ(0x0123, i16);
	EXPECT_EQ(0x0123, u16);
	CPLUS_DATA_REVERSE(&i16);
	CPLUS_DATA_REVERSE(&u16);
	EXPECT_EQ(0x2301, i16);
	EXPECT_EQ(0x2301, u16);
	EXPECT_EQ(0x01234567, i32);
	EXPECT_EQ(0x01234567, u32);
	CPLUS_DATA_REVERSE(&i32);
	CPLUS_DATA_REVERSE(&u32);
	EXPECT_EQ(0x67452301, i32);
	EXPECT_EQ(0x67452301, u32);
	EXPECT_EQ(0x0123456789abcdef, i64);
	EXPECT_EQ(0x0123456789abcdef, u64);
	CPLUS_DATA_REVERSE(&i64);
	CPLUS_DATA_REVERSE(&u64);
	EXPECT_EQ(0xefcdab8967452301, i64);
	EXPECT_EQ(0xefcdab8967452301, u64);
}

TEST(cplus_data_new_ex, all_type)
{
	int32_t v1 = 123456;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_bool_ex(true, strlen("test_bool"), "test_bool")));
	EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int8_ex(0xab, strlen("test_int8"), "test_int8")));
	EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int16_ex(0xabcd, strlen("test_int16"), "test_int16")));
	EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32_ex(123456, strlen("test_int32"), "test_int32")));
	EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int64_ex(0x0123abcddcba3210, strlen("test_int64"), "test_int64")));
	EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint8_ex(0xab, strlen("test_uint8"), "test_uint8")));
	EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint16_ex(0xabcd, strlen("test_uint16"), "test_uint16")));
	EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint32_ex(123456, strlen("test_uint32"), "test_uint32")));
	EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint64_ex(0x0123abcddcba3210, strlen("test_uint64"), "test_uint64")));
	EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_float_ex(3.1415926535, strlen("test_float"), "test_float")));
	EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_double_ex(3.1415926535,strlen("test_double"), "test_double")));
	EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_pointer_ex(&v1, strlen("test_pointer"), "test_pointer")));
	EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_string_ex(strlen("Hello World"), (char *)("Hello World"), strlen("test_string"), "test_string")));
	EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new_ex, thread_safe)
{
	int32_t v1 = 123456;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_bool_ex(true, strlen("test_bool"), "test_bool")));
	EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int8_ex(0xab, strlen("test_int8"), "test_int8")));
	EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int16_ex(0xabcd, strlen("test_int16"), "test_int16")));
	EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32_ex(123456, strlen("test_int32"), "test_int32")));
	EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int64_ex(0x0123abcddcba3210, strlen("test_int64"), "test_int64")));
	EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint8_ex(0xab, strlen("test_uint8"), "test_uint8")));
	EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint16_ex(0xabcd, strlen("test_uint16"), "test_uint16")));
	EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint32_ex(123456, strlen("test_uint32"), "test_uint32")));
	EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint64_ex(0x0123abcddcba3210, strlen("test_uint64"), "test_uint64")));
	EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_float_ex(3.1415926535, strlen("test_float"), "test_float")));
	EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_double_ex(3.1415926535, strlen("test_double"), "test_double")));
	EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_pointer_ex(&v1, strlen("test_pointer"), "test_pointer")));
	EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_string_ex(strlen("Hello World"), (char *)("Hello World"), strlen("test_string"), "test_string")));
	EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_BOOL_THREAD_SAFE)
{
	bool result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_bool_ex(true, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool"), "test_bool"));
	EXPECT_EQ(0, strcmp("test_bool", cplus_data_get_key(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_bool_bool_bool_bool"), "test_bool_bool_bool_bool"));
	EXPECT_EQ(0, strcmp("test_bool_bool_bool_bool", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(bool), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_BOOL, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(bool), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ(CPLUS_TRUE, result);
	EXPECT_EQ(0, cplus_data_set_bool(data, false));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_get_bool(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT8_THREAD_SAFE)
{
	int8_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int8_ex(0xab, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int8"), "test_int8"));
	EXPECT_EQ(0, strcmp("test_int8", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT8, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int8_t)(0xab), result);
	EXPECT_EQ(0, cplus_data_set_int8(data, 0xba));
	EXPECT_EQ((int8_t)(0xba), cplus_data_get_int8(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT16_THREAD_SAFE)
{
	int16_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int16_ex(0xabcd, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int16"), "test_int16"));
	EXPECT_EQ(0, strcmp("test_int16", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT16, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int16_t)(0xabcd), result);
	EXPECT_EQ(0, cplus_data_set_int16(data, 0xdcba));
	EXPECT_EQ((int16_t)(0xdcba), cplus_data_get_int16(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT32_THREAD_SAFE)
{
	int32_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32_ex(123456, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int32"), "test_int32"));
	EXPECT_EQ(0, strcmp("test_int32", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT32, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int32_t)(123456), result);
	EXPECT_EQ(0, cplus_data_set_int32(data, -123456));
	EXPECT_EQ((int32_t)(-123456), cplus_data_get_int32(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_INT64_THREAD_SAFE)
{
	int64_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int64_ex(0x0123abcddcba3210, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_int64"), "test_int64"));
	EXPECT_EQ(0, strcmp("test_int64", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_INT64, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(int64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int64_t)(0x0123abcddcba3210), result);
	EXPECT_EQ(0, cplus_data_set_int64(data, 0x7FFFFFFFFFFFFFFF));
	EXPECT_EQ((int64_t)(0x7FFFFFFFFFFFFFFF), cplus_data_get_int64(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT8_THREAD_SAFE)
{
	uint8_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint8_ex(0xab, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint8"), "test_uint8"));
	EXPECT_EQ(0, strcmp("test_uint8", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT8, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint8_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint8_t)(0xab), result);
	EXPECT_EQ(0, cplus_data_set_uint8(data, 0xba));
	EXPECT_EQ((uint8_t)(0xba), cplus_data_get_uint8(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT16_THREAD_SAFE)
{
	uint16_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint16_ex(0xabcd, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint16"), "test_uint16"));
	EXPECT_EQ(0, strcmp("test_uint16", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT16, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint16_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint16_t)(0xabcd), result);
	EXPECT_EQ(0, cplus_data_set_uint16(data, 0xdcba));
	EXPECT_EQ((uint16_t)(0xdcba), cplus_data_get_uint16(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT32_THREAD_SAFE)
{
	uint32_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint32_ex(123456, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint32"), "test_uint32"));
	EXPECT_EQ(0, strcmp("test_uint32", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT32, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint32_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint32_t)(123456), result);
	EXPECT_EQ(0, cplus_data_set_uint32(data, 6543210));
	EXPECT_EQ((uint32_t)(6543210), cplus_data_get_uint32(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_UINT64_THREAD_SAFE)
{
	uint64_t result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_uint64_ex(0x0123abcddcba3210, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_uint64"), "test_uint64"));
	EXPECT_EQ(0, strcmp("test_uint64", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_UINT64, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(uint64_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((uint64_t)(0x0123abcddcba3210), result);
	EXPECT_EQ(0, cplus_data_set_uint64(data, 0xdcba32100123abcd));
	EXPECT_EQ((uint64_t)(0xdcba32100123abcd), cplus_data_get_uint64(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_FLOAT_THREAD_SAFE)
{
	float result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_float_ex(3.1415926535, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_float"), "test_float"));
	EXPECT_EQ(0, strcmp("test_float", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(float), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_FLOAT, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(float), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - result) < 0.0000001f);
	// EXPECT_EQ(0, result - 3.14159);
	EXPECT_EQ(0, cplus_data_set_float(data, 2.7182818284));
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_float(data)) < 0.01f);
	// EXPECT_EQ(0, cplus_data_get_float(data) - 2.71828);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_DOUBLE_THREAD_SAFE)
{
	double result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_double_ex(3.1415926535, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_double"), "test_double"));
	EXPECT_EQ(0, strcmp("test_double", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(double), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_DOUBLE, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(double), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - result) < 0.0000001f);
	// EXPECT_EQ(0, result - 3.14159);
	EXPECT_EQ(0, cplus_data_set_double(data, 2.7182818284));
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_double(data)) < 0.0000001f);
	// EXPECT_EQ(0, cplus_data_get_double(data) - 2.71828);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_POINTER_THREAD_SAFE)
{
	int32_t v1 = 123456, v2 = 654321;
	void * result;
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_pointer_ex(&v1, 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_pointer"), "test_pointer"));
	EXPECT_EQ(0, strcmp("test_pointer", cplus_data_get_key(data)));
	EXPECT_EQ(sizeof(intptr_t), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_POINTER, cplus_data_get_type(data));
	EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int32_t)(123456), *((int32_t *)result));
	EXPECT_EQ(0, cplus_data_set_pointer(data, &v2));
	EXPECT_EQ(sizeof(intptr_t), cplus_data_get_value(data, &result, CPLUS_NULL));
	EXPECT_EQ((int32_t)(654321), *((int32_t *)result));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_new, CPLUS_DATA_TYPE_STRING_THREAD_SAFE)
{
	char bufs[32];
	uint32_t len = sizeof(bufs);
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_string_ex(strlen("Hello World"), (char *)("Hello World"), 0, CPLUS_NULL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_key(data, strlen("test_string"), "test_string"));
	EXPECT_EQ(0, strcmp("test_string", cplus_data_get_key(data)));
	EXPECT_EQ(strlen("Hello World"), cplus_data_get_data_size(data));
	EXPECT_EQ(CPLUS_DATA_TYPE_STRING, cplus_data_get_type(data));
	EXPECT_EQ(strlen("Hello World"), cplus_data_get_value(data, &len, bufs));
	EXPECT_EQ(0, strcmp("Hello World", bufs));
	EXPECT_EQ(0, cplus_data_set_string(data, strlen("AaBbCcDd"), (char *)("AaBbCcDd")));
	EXPECT_EQ(strlen("AaBbCcDd"), cplus_data_get_data_size(data));
	EXPECT_EQ(strlen("AaBbCcDd"), strlen(cplus_data_get_string(data)));
	EXPECT_EQ(0, strcmp("AaBbCcDd", cplus_data_get_string(data)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_set_action_mode, functionity)
{
	cplus_data data = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data = cplus_data_new_int32(123456)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_set_action_mode(data, 654321));
	EXPECT_EQ(654321, cplus_data_get_action_mode(data));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_create_group_node, functionity)
{
	cplus_data group_node = CPLUS_NULL;
	cplus_llist group = CPLUS_NULL;
	char test[] = "test";
	int32_t i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (group_node = cplus_data_create_group_node(test)));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_data_check(group_node)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (group = cplus_data_get_group(group_node)));
	EXPECT_EQ(CPLUS_TRUE, (true == cplus_llist_check(group)));
	EXPECT_EQ(0, strcmp(test, cplus_data_get_key(group_node)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i3));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i4));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_llist_push_back(group, &i5));
	EXPECT_EQ(i1, *((int32_t *)cplus_llist_pop_front(group)));
	EXPECT_EQ(i2, *((int32_t *)cplus_llist_pop_front(group)));
	EXPECT_EQ(i3, *((int32_t *)cplus_llist_pop_front(group)));
	EXPECT_EQ(i4, *((int32_t *)cplus_llist_pop_front(group)));
	EXPECT_EQ(i5, *((int32_t *)cplus_llist_pop_front(group)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete_group_node(group_node));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_data_clone_value, functionity)
{
	cplus_data data1 = CPLUS_NULL, data2 = CPLUS_NULL;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_bool(true)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_bool(false)));
	EXPECT_EQ(CPLUS_TRUE, cplus_data_get_bool(data1));
	EXPECT_EQ(CPLUS_FALSE, cplus_data_get_bool(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ(CPLUS_TRUE, true == cplus_data_get_bool(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_int8(0xAB)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_int8(0xCD)));
	EXPECT_EQ((int8_t)(0xAB), cplus_data_get_int8(data1));
	EXPECT_EQ((int8_t)(0xCD), cplus_data_get_int8(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((int8_t)(0xAB), cplus_data_get_int8(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_uint8(0xAB)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_uint8(0xCD)));
	EXPECT_EQ((uint8_t)(0xAB), cplus_data_get_uint8(data1));
	EXPECT_EQ((uint8_t)(0xCD), cplus_data_get_uint8(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((uint8_t)(0xAB), cplus_data_get_uint8(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_int16(0x1234)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_int16(0x5678)));
	EXPECT_EQ((int16_t)(0x1234), cplus_data_get_int16(data1));
	EXPECT_EQ((int16_t)(0x5678), cplus_data_get_int16(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((int16_t)(0x1234), cplus_data_get_int16(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_uint16(0x1234)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_uint16(0x5678)));
	EXPECT_EQ((uint16_t)(0x1234), cplus_data_get_uint16(data1));
	EXPECT_EQ((uint16_t)(0x5678), cplus_data_get_uint16(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((uint16_t)(0x1234), cplus_data_get_uint16(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_int32(0x12345678)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_int32(0x5678abcd)));
	EXPECT_EQ((int32_t)(0x12345678), cplus_data_get_int32(data1));
	EXPECT_EQ((int32_t)(0x5678abcd), cplus_data_get_int32(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((int32_t)(0x12345678), cplus_data_get_int32(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_uint32(0x12345678)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_uint32(0x5678abcd)));
	EXPECT_EQ((uint32_t)(0x12345678), cplus_data_get_uint32(data1));
	EXPECT_EQ((uint32_t)(0x5678abcd), cplus_data_get_uint32(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((uint32_t)(0x12345678), cplus_data_get_uint32(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_int64(0x0123abcddcba3210)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_int64(0xabcdef0123456789)));
	EXPECT_EQ((int64_t)(0x0123abcddcba3210), cplus_data_get_int64(data1));
	EXPECT_EQ((int64_t)(0xabcdef0123456789), cplus_data_get_int64(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((int64_t)(0x0123abcddcba3210), cplus_data_get_int64(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_uint64(0x0123abcddcba3210)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_uint64(0xabcdef0123456789)));
	EXPECT_EQ((uint64_t)(0x0123abcddcba3210), cplus_data_get_uint64(data1));
	EXPECT_EQ((uint64_t)(0xabcdef0123456789), cplus_data_get_uint64(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ((uint64_t)(0x0123abcddcba3210), cplus_data_get_uint64(data2));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_float(3.1415926535)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_float(2.7182818284)));
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - cplus_data_get_float(data1)) < 0.01f);
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_float(data2)) < 0.01f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - cplus_data_get_float(data2)) < 0.01f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_double(2.7182818284)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_double(3.1415926535)));
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_double(data1)) < 0.0000001f);
	EXPECT_EQ(CPLUS_TRUE, fabs(3.1415926535 - cplus_data_get_double(data2)) < 0.0000001f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data2, data1));
	EXPECT_EQ(CPLUS_TRUE, fabs(2.7182818284 - cplus_data_get_double(data2)) < 0.0000001f);
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data1 = cplus_data_new_string(strlen("Hello World"), (char *)("Hello World"))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (data2 = cplus_data_new_string(strlen("AaBbCcDd 01234567"), (char *)("AaBbCcDd 01234567"))));
	EXPECT_EQ(0, strcmp("Hello World", cplus_data_get_string(data1)));
	EXPECT_EQ(0, strcmp("AaBbCcDd 01234567", cplus_data_get_string(data2)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_clone_value(data1, data2));
	EXPECT_EQ(0, strcmp("AaBbCcDd 01234567", cplus_data_get_string(data1)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data1));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_data_delete(data2));
	EXPECT_EQ(0, cplus_mgr_report());
}