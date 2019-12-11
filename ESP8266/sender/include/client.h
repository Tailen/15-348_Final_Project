/*
 * client.h
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 * 
 * HTTP client
 */

#ifndef APP_INCLUDE_CLIENT_H_
#define APP_INCLUDE_CLIENT_H_

#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"
#include "ets_sys.h"
#include "ip_addr.h"
#include "espconn.h"

// extern struct espconn *pEspConn;
#define post_header "POST /client?command=%s HTTP/1.1\r\nUser-Agent: curl/7.37.0\r\nHost: 192.168.4.1\r\nAccept: */*\r\n\r\n"

struct espconn user_tcp_conn;
void my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port);
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg);
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err);

#endif /* APP_INCLUDE_CLIENT_H_ */
