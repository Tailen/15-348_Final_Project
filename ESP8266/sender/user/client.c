/*
 * client.c
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 * 
 * HTTP client
 */

#include "client.h"

struct espconn user_tcp_conn;
extern uint32 current_command;

void ICACHE_FLASH_ATTR
user_tcp_sent_cb(void *arg)
{
	os_printf("TCP package sent successfully!\n");
	// Disconnect from server
	struct espconn *pespconn = arg;
	espconn_disconnect(pespconn);
}

void ICACHE_FLASH_ATTR 
user_tcp_discon_cb(void *arg)
{
	os_printf("TCP disconnected\n");
}

void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pdata, unsigned short len)
{
	os_printf("TCP package received\n");
}

void ICACHE_FLASH_ATTR
user_tcp_recon_cb(void *arg, sint8 err)
{
	os_printf("TCP reconnection callback, error code: %d\n", err);
}

void ICACHE_FLASH_ATTR
user_tcp_connect_cb(void *arg)
{
	os_printf("TCP connection established\n");
	struct espconn *pespconn = arg;
	char *request_buf = (char *)os_zalloc(128);
	// Callbacks
	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

	// Send command
	switch (current_command)
	{
	case 0:
		os_sprintf(request_buf, post_header, "c0");
		break;
	case 1:
		os_sprintf(request_buf, post_header, "c1");
		break;
	case 2:
		os_sprintf(request_buf, post_header, "c2");
		break;
	case 3:
		os_sprintf(request_buf, post_header, "c3");
		break;
	}
	espconn_sent(pespconn, request_buf, os_strlen(request_buf));
	free(request_buf);
}

void ICACHE_FLASH_ATTR
my_station_init(struct ip_addr *remote_ip, struct ip_addr *local_ip, int remote_port)
{
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;
	user_tcp_conn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4);
	os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4);
	user_tcp_conn.proto.tcp->local_port = espconn_port();
	user_tcp_conn.proto.tcp->remote_port = remote_port;

	// Register connect and reconnect callbacks
	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);
}
