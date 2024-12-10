#include "internal.h"
#define WRITTEN_COUNT 5
#define RUN_COUNT 10

int32_t verification_count = 0, callback_param = 0;

int32_t on_read(int32_t fd, int32_t value, void * cb_param)
{
	verification_count += 5;
	return 0;
}

int32_t on_read_ex(int32_t fd, int32_t value, void * cb_param)
{
	int32_t * param = (int32_t *)(cb_param);
	verification_count += 5;
	(* param ) += 5;
	return 0;
}

TEST(cplus_event_server_new, functionity)
{
	cplus_event_server server = CPLUS_NULL;
	cplus_event_client client = CPLUS_NULL;
	CPLUS_EVENT_SERVER_CB_FUNCS_T on_funcs = {0};
	verification_count = 0;

	on_funcs.on_read = on_read;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (server = cplus_event_server_new(
		0, CPLUS_EVENT_SERVER_FLAG_NONBLOCK, &on_funcs))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_start(server));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (client = cplus_event_client_attach(server))));
	for (int32_t i = 0; i < RUN_COUNT; i++)
	{
		EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_add_count(client, WRITTEN_COUNT));
		cplus_systime_sleep_msec(1000);
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_delete(client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_delete(server));
	EXPECT_EQ(CPLUS_TRUE, ((RUN_COUNT * WRITTEN_COUNT) == verification_count));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(cplus_event_server_new, CPLUS_SOCKET_TYPE_STREAM_LOCAL)
{
	cplus_event_server server = CPLUS_NULL;
	cplus_event_client client = CPLUS_NULL;
	struct cplus_event_server_config config = {0};
	CPLUS_EVENT_SERVER_CB_FUNCS_T on_funcs = {0};
	verification_count = 0;

	config.enable_ipc = true;
	config.init_val = 0;
	config.flag = CPLUS_EVENT_SERVER_FLAG_NONBLOCK;
	config.cb_param = &callback_param;
	on_funcs.on_read = on_read_ex;

	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (server = cplus_event_server_new_config(
		&config, &on_funcs))));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_start(server));
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (client = cplus_event_client_connect())));
	for (int32_t i = 0; i < RUN_COUNT; i++)
	{
		EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_add_count(client, WRITTEN_COUNT));
		cplus_systime_sleep_msec(1000);
	}
	EXPECT_EQ(CPLUS_SUCCESS, cplus_event_client_delete(client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_event_server_delete(server));
	EXPECT_EQ(CPLUS_TRUE, ((RUN_COUNT * WRITTEN_COUNT) == verification_count));
	EXPECT_EQ(CPLUS_TRUE, ((RUN_COUNT * WRITTEN_COUNT) == callback_param));
	EXPECT_EQ(0, cplus_mgr_report());
}