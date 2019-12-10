/*
 * client.h
 *
 *  Created on: 2016年11月12日
 *      Author: Administrator
 */

#ifndef APP_INCLUDE_CLIENT_H_
#define APP_INCLUDE_CLIENT_H_
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"

ETSTimer connect_timer;// 应用层心跳包定时器
LOCAL bool checkback = false; //回执

extern bool SendEN_Flag;
extern struct espconn *pEspConn;

struct espconn user_tcp_conn;
void my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port);
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg);
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err);

void ICACHE_FLASH_ATTR TCP_conned(struct espconn *pespconn);


//定义数据类型


#endif /* APP_INCLUDE_CLIENT_H_ */
