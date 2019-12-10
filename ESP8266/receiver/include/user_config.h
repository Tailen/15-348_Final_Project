/*
 * user_config.h
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define SOFTAP_ENCRYPT
// #define CONNECT_KNOWN_AP
// #define CONNECT_KNOWN_SERVER

#ifdef SOFTAP_ENCRYPT
#define SOFTAP_SSID "ESP8266"
#define SOFTAP_PASSWD "password"
#endif

#ifdef CONNECT_KNOWN_AP
#define AP_SSID "ESP8266"
#define AP_PASSWD "password"
#endif

#ifdef CONNECT_KNOWN_SERVER
const uint8 remote_ip[4] = {122,114,122,174};
const int remote_connect_port = 36973;
#endif

#endif /* __USER_CONFIG_H__ */
