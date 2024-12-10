#include "internal.h"
#define SERVER_NAME "unittest_server"
#define CLIENT_NAME "unittest_client"
#define SERVER_PORT 12345
#define CLIENT_PORT 54321
#define TEST_FILE   "/var/tmp/test"
#define TEST_STR	"Hello World"
#define INVALID_FD -1

TEST(CPLUS_SOCKET_TYPE_DGRAM_LOCAL, functionity)
{
	cplus_socket skt_server = CPLUS_NULL, skt_client = CPLUS_NULL;
	char data_bufs[100] = {0}, from_addr[64] = {0};
	int32_t from_port = 0;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, SERVER_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_client, CLIENT_NAME, CLIENT_PORT));
	EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_sendto(skt_client
		, (void *)("AaBbCcDd"), strlen("AaBbCcDd") + 1, SERVER_NAME, SERVER_PORT));
	EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_recvfrom(skt_server
		, data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(CLIENT_NAME, from_addr));
	EXPECT_EQ(0, strcmp(data_bufs, "AaBbCcDd"));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_sendto(skt_client, (void *)("Hello World"), strlen("Hello World") + 1, SERVER_NAME, SERVER_PORT));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recvfrom(skt_server, data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(CLIENT_NAME, from_addr));
	EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(CPLUS_SOCKET_TYPE_STREAM_LOCAL, functionity)
{
	cplus_socket skt_server = CPLUS_NULL, skt_client = CPLUS_NULL, skt_remote = CPLUS_NULL;
	char data_bufs[100] = {0};
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_remote = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_remote, SERVER_NAME, 0));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_send(skt_remote, (void *)("Hello World"), strlen("Hello World") + 1));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recv(skt_client, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
	EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_send(skt_client, (void *)("0123456789"), strlen("0123456789") + 1));
	EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_recv(skt_remote, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(data_bufs, "0123456789"));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

TEST(CPLUS_SOCKET_TYPE_UDP_IPV4, functionity)
{
	cplus_socket skt_server = CPLUS_NULL, skt_client = CPLUS_NULL;
	char data_bufs[100] = {0}, from_addr[64] = {0};
	int32_t from_port = 0;
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_UDP_IPV4)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_UDP_IPV4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, "127.0.0.1", SERVER_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_client, "127.0.0.1", CLIENT_PORT));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_sendto(skt_client
		, (void *)("Hello World"), strlen("Hello World") + 1, "127.0.0.1", SERVER_PORT));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recvfrom(skt_server
		, data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
	EXPECT_EQ(0, strcmp(from_addr, "127.0.0.1"));
	EXPECT_EQ(CPLUS_TRUE, (from_port == CLIENT_PORT));
	EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_sendto(skt_server, (void *)("AaBbCcDd"), strlen("AaBbCcDd") + 1, "127.0.0.1", CLIENT_PORT));
	EXPECT_EQ(strlen("AaBbCcDd") + 1, cplus_socket_recvfrom(skt_client, data_bufs, sizeof(data_bufs), from_addr, sizeof(from_addr), &from_port, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(data_bufs, "AaBbCcDd"));
	EXPECT_EQ(0, strcmp(from_addr, "127.0.0.1"));
	EXPECT_EQ(CPLUS_TRUE, (from_port == SERVER_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(0, cplus_mgr_report());
}

TEST(CPLUS_SOCKET_TYPE_TCP_IPV4, functionity)
{
	cplus_socket skt_server = CPLUS_NULL, skt_client = CPLUS_NULL, skt_remote = CPLUS_NULL;
	char data_bufs[100] = {0};
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_TCP_IPV4)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_remote = cplus_socket_new(CPLUS_SOCKET_TYPE_TCP_IPV4)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_setopt_reuse_addr(skt_server, true));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, "127.0.0.1", SERVER_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_remote, "127.0.0.1", CLIENT_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_remote, "127.0.0.1", SERVER_PORT));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_send(skt_remote, (void *)("Hello World"), strlen("Hello World") + 1));
	EXPECT_EQ(strlen("Hello World") + 1, cplus_socket_recv(skt_client, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(data_bufs, "Hello World"));
	EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_send(skt_client, (void *)("0123456789"), strlen("0123456789") + 1));
	EXPECT_EQ(strlen("0123456789") + 1, cplus_socket_recv(skt_remote, data_bufs, sizeof(data_bufs), CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(0, strcmp(data_bufs, "0123456789"));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

TEST(cplus_socket_send_fd, CPLUS_SOCKET_TYPE_STREAM_LOCAL)
{
	cplus_socket skt_server = CPLUS_NULL, skt_client = CPLUS_NULL, skt_remote = CPLUS_NULL;
	cplus_file test_file = CPLUS_NULL;
	int32_t recv_fd = INVALID_FD, send_fd = INVALID_FD;
	char rr[32] = {0};
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_file = cplus_file_new((char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_write(test_file, strlen(TEST_STR), (void *)(TEST_STR)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_reset_pos(test_file));
	EXPECT_EQ(CPLUS_TRUE, (INVALID_FD != (send_fd = cplus_file_get_fd(test_file))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_client, SERVER_NAME, 0));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_remote = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_send_fd(skt_remote, send_fd, CPLUS_NULL, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_recv_fd(skt_client, &recv_fd, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(strlen(TEST_STR), read(recv_fd, rr, sizeof(rr)));
	EXPECT_EQ(CPLUS_TRUE, (0 == strcmp(TEST_STR, rr)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(test_file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

TEST(cplus_socket_send_fd, SERVER)
{
	cplus_socket skt_server = CPLUS_NULL, skt_remote = CPLUS_NULL;
	cplus_file test_file = CPLUS_NULL;
	int32_t send_fd = INVALID_FD;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_file = cplus_file_new((char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_write(test_file, strlen(TEST_STR), (void *)(TEST_STR)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_reset_pos(test_file));
	EXPECT_EQ(CPLUS_TRUE, (INVALID_FD != (send_fd = cplus_file_get_fd(test_file))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_listen(skt_server, 10));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_remote = cplus_socket_accept(skt_server, CPLUS_INFINITE_TIMEOUT)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_send_fd(skt_remote, send_fd, CPLUS_NULL, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(test_file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_remote));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

TEST(cplus_socket_recv_fd, CLIENT)
{
	cplus_socket skt_client = CPLUS_NULL;
	int32_t recv_fd = INVALID_FD;
	char rr[32] = {0};
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_STREAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_connect(skt_client, SERVER_NAME, 0));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_recv_fd(skt_client, &recv_fd, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(strlen(TEST_STR), read(recv_fd, rr, sizeof(rr)));
	EXPECT_EQ(CPLUS_TRUE, (0 == strcmp(TEST_STR, rr)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_mgr_report());
}

TEST(cplus_socket_send_fd, CPLUS_SOCKET_TYPE_DGRAM_LOCAL)
{
	cplus_socket skt_server = CPLUS_NULL, skt_client = CPLUS_NULL;
	cplus_file test_file = CPLUS_NULL;
	char rr[32] = {0};
	int32_t recv_fd = INVALID_FD, send_fd = INVALID_FD;
	EXPECT_EQ(CPLUS_TRUE, (CPLUS_NULL != (test_file = cplus_file_new((char *)(TEST_FILE), CPLUS_FILE_ACCESS_RDWR))));
	EXPECT_EQ(strlen(TEST_STR), cplus_file_write(test_file, strlen(TEST_STR), (void *)(TEST_STR)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_reset_pos(test_file));
	EXPECT_EQ(CPLUS_TRUE, (INVALID_FD != (send_fd = cplus_file_get_fd(test_file))));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_server = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
	EXPECT_EQ(CPLUS_TRUE, CPLUS_NULL != (skt_client = cplus_socket_new(CPLUS_SOCKET_TYPE_DGRAM_LOCAL)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_server, SERVER_NAME, SERVER_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_bind(skt_client, CLIENT_NAME, CLIENT_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_send_fd(skt_server, send_fd, CLIENT_NAME, CLIENT_PORT));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_recv_fd(skt_client, &recv_fd, CPLUS_INFINITE_TIMEOUT));
	EXPECT_EQ(strlen(TEST_STR), read(recv_fd, rr, sizeof(rr)));
	EXPECT_EQ(CPLUS_TRUE, (0 == strcmp(TEST_STR, rr)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_delete(test_file));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_file_remove((char *)(TEST_FILE)));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_server));
	EXPECT_EQ(CPLUS_SUCCESS, cplus_socket_delete(skt_client));
	EXPECT_EQ(0, cplus_mgr_report());
}