/*
 * user_main.h
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 */

#ifndef __INCLUDE_USER_MAIN_H__
#define __INCLUDE_USER_MAIN_H__

// #include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "ip_addr.h"

// #include "client.h"
#include "user_config.h"

void os_delay_ms(uint16 ms);
void ICACHE_FLASH_ATTR config_ap_and_sta_cb(void);
void ICACHE_FLASH_ATTR wifi_event_handler_cb(System_Event_t *event);
void ICACHE_FLASH_ATTR connect_ap(void *arg, STATUS status);
void ICACHE_FLASH_ATTR check_ap_connected(void *arg);

os_timer_t ap_connect_timer; // A software timer, less accurate than HW timer

#endif /* __INCLUDE_USER_MAIN_H__ */
