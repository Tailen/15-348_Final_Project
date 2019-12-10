/*
 * client.c
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 */

#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "eagle_soc.h"
#include "espconn.h"

#include "client.h"
#include "user_main.h"

uint8 beacon_data[6] = {0x00,0x04,0x00,0x02,0x7F,0x7F};

void ICACHE_FLASH_ATTR
my_station_init(struct ip_addr *remote_ip, struct ip_addr *local_ip, int remote_port)
{
	struct espconn user_tcp_conn;
	os_memset(&user_tcp_conn, 0, sizeof(struct espconn)); // zero out everything to prevent garbage values
	// espconn configuration
	user_tcp_conn.type = ESPCONN_TCP; // set connection type to TPC
	user_tcp_conn.state = ESPCONN_NONE; // set state
	user_tcp_conn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp)); // alloc memory and zero it out
	os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4); // set local ip
	os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4); // set remote ip
	user_tcp_conn.proto.tcp->local_port = espconn_port(); // set local port
	user_tcp_conn.proto.tcp->remote_port = remote_port; // set remote port

	// To-do: use espconn secure protocol
	// Register connection success and fail callback functions
	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);

	os_printf("Registered connect and reconnect callbacks\r\n");
	// Start connection attempt
	espconn_connect(&user_tcp_conn);
}

/*
 * Callback funcution after successful connection to remote server
 */
void ICACHE_FLASH_ATTR 
user_tcp_connect_cb(void *arg)
{
	os_printf("Successfully connect to server!!!\r\n");

	struct espconn *pespconn = arg;
	espconn_regist_recvcb(pespconn, user_tcp_recv_cb); // Received data callback
	espconn_regist_sentcb(pespconn, user_tcp_sent_cb); // Sent data callback
	espconn_regist_disconcb(pespconn, user_tcp_discon_cb); // Disconnect callback

	os_timer_disarm(&keeplive_timer);
	os_timer_setfn(&keeplive_timer, (os_timer_func_t *)TCP_connected, pespconn);
	os_timer_arm(&keeplive_timer, 30000, 1); // Send a keepalive package every 30 seconds
	wait_reponse = false;
	// First message
	uint8 uart_buf[128] = {0x00,0x0A,0x00,0x01,0x00,0x64,0x00,0xFA,0x00,0x32,0x00,0x02};
	espconn_send(pespconn, uart_buf, 12);
}

/**
 * TCP 心跳包函数，通过应用层检测心跳包
 * 判断是否喂狗，喂狗则根据心跳周期发送心跳包
 * 若未喂狗，则 watchdog_count 次数后 重新连接
 */
void ICACHE_FLASH_ATTR 
TCP_connected(struct espconn *pespconn)
{
	if (wait_reponse) {
		if (watchdog_count >= 3) {
			watchdog_count = 0;
			os_printf("No response from server for 3 times, reconnecting...\r\n");
			espconn_disconnect(pespconn);
		} else {
			watchdog_count++;
			os_printf("No response from server for %u times\r\n", watchdog_count);
		}
	} else { // 发送心跳包
		espconn_send(pespconn, beacon_data, 6);
		wait_reponse = true;
		watchdog_count = 0;
	}
}

/*
 * Callback funcution after failed connection to remote server
 */
void ICACHE_FLASH_ATTR 
user_tcp_recon_cb(void *arg, sint8 err)
{
	os_printf("user_tcp_recon_cb -> connection failed, error code: %d\... Retrying...\r\n", err);
	// Print local ip info
	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig);
	os_printf("Current local_ip:" IPSTR ", local_gw:" IPSTR ", remote_netmask:" IPSTR,
			  IP2STR(&ipconfig.ip), IP2STR(&ipconfig.gw), IP2STR(&ipconfig.netmask));
	os_timer_disarm(&keeplive_timer); // 断开连接，未连接上无需发送心跳包
	// Re-attempt connection
	espconn_connect((struct espconn *)arg);
}

/**
 * 设备长连接，若对方原因导致断线，因重新拨号。
 * 实验1: server中断连接，ESP收到消息，因重新拨号
 */
void ICACHE_FLASH_ATTR 
user_tcp_discon_cb(void *arg)
{
	os_printf("Disconnect successful!!!\r\n");
	os_timer_disarm(&keeplive_timer); // 断开连接，未连接上无需发送心跳包
	// Re-attempt connection
	espconn_connect((struct espconn *)arg);
}

/*
 * Sent data callback
 */
void ICACHE_FLASH_ATTR 
user_tcp_sent_cb(void *arg)
{
	os_printf("Data sent successful!!!\r\n");
}

/**
 * Tcp收到数据：1.喂狗
 *             2.心跳包数据
 *             3. 业务逻辑数据
 */
void ICACHE_FLASH_ATTR 
user_tcp_recv_cb(void *arg, char *pdata, unsigned short len)
{
	os_printf("Received data \"%s\" with length %u\r\n", pdata, len);

 	// uart0_tx_buffer(pdata,len);

	//	os_printf("%s", &pdata[0]);
	//  os_printf("#### %2X %2X %X %X %X %X",(&pdata[0])[0],(&pdata[0])[1],(&pdata[0])[2],(&pdata[0])[3],(&pdata[0])[4],(&pdata[0])[5]);
	//	if (strcmp(pdata,"7F7F") == 0) {
	//		os_printf("7F7F收到数据：%s\r\n", pdata);
	//	} else {
	//		os_printf("F7F7收到数据：%s\r\n", pdata);
	//	}
	//	os_printf("F7F7收到数据：%s\r\n", pdata);
	//	uart0_sendStr(convertPData); //应该放在心跳包另外一个分之中

	wait_reponse = false; //心跳包喂狗
}
