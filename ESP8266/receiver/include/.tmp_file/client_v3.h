/*
 * client.h
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 */

#ifndef __INCLUDE_USER_H__
#define __INCLUDE_USER_H__

void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port);
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg);
void ICACHE_FLASH_ATTR TCP_connected(struct espconn *pespconn);
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err);
void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg);
void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg);
void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg, char *pdata, unsigned short len);

os_timer_t keeplive_timer;
static bool wait_reponse = false; // If the module is expecting a reponse for heart_buf
static uint8 watchdog_count = 0; // Number of heartbuf with no response

/*


//*****************************************************************************
//
// Make sure all of the definitions in this header have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

// client functions here

#ifdef __cplusplus
}
#endif


*/


#endif /* __INCLUDE_USER_H__ */
