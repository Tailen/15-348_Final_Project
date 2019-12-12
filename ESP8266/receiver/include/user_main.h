/*
 * user_main.h
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 */

#ifndef __INCLUDE_USER_MAIN_H__
#define __INCLUDE_USER_MAIN_H__

#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"

#include "user_config.h"
#include "user_webserver.h"

void os_delay_ms(uint16 ms);
void ICACHE_FLASH_ATTR config_ap_and_sta_cb(void);
void ICACHE_FLASH_ATTR user_GPIO_init(void);
void ICACHE_FLASH_ATTR user_webserver_init(uint32 port);
void ICACHE_FLASH_ATTR wifi_event_handler_cb(System_Event_t *event);

os_timer_t ap_connect_timer; // A software timer, less accurate than HW timer
os_timer_t gpio_check_timer;

#endif /* __INCLUDE_USER_MAIN_H__ */
