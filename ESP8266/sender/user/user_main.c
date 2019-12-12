/*
 * user_main.c
 * 
 * Created on: 27 May 2019
 * Author: Hao Mei
 */

#include "user_main.h"

/* Default pre-compilation macros for intellisense */
#ifdef __INTELLISENSE__
    #define SPI_FLASH_SIZE_MAP 2
#endif

const uint8 remote_ip[4] = {192,168,4,1};
const int remote_connect_port = 80;
struct espconn user_tcp_conn;
uint8 last_command = 0;
uint8 current_command = 0;

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#else
#error "The flash map is not supported"
#endif

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if (!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}

/*
 * App entry point 
 */
void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(115200, 115200); // set uart baud rate
    wifi_set_opmode(STATION_MODE); // set mode to station mode
    os_delay_ms(10); // wait for initial settings to finish

    // Event handler callbacks
    wifi_set_event_handler_cb(wifi_event_handler_cb); // monitor wifi state
    system_init_done_cb(config_ap_and_sta_cb); // init done callback
}

/*
 * Configure softAP and connect to AP
 */
void ICACHE_FLASH_ATTR
config_ap_and_sta_cb(void)
{
    os_printf("user_init done\r\n");

    // Set AP ssid and password
    #ifdef SOFTAP_ENCRYPT
    struct softap_config ap_config;
    wifi_softap_get_config(&ap_config);
    os_memcpy(ap_config.ssid, SOFTAP_SSID, os_strlen(SOFTAP_SSID)+1);
    os_memcpy(ap_config.password, SOFTAP_PASSWD, os_strlen(SOFTAP_PASSWD)+1);
    ap_config.ssid_len = os_strlen(SOFTAP_SSID); // set ssid length
    ap_config.authmode = AUTH_WPA_WPA2_PSK; // set authentication mode 
    wifi_softap_set_config(&ap_config); // save settings to flash
    #endif

    // Scan all AP, then invoke callback connect_ap
    wifi_station_scan(NULL, connect_ap);
}

/* 
 * Connect to AP using results from wifi_station_scan
 */
void ICACHE_FLASH_ATTR
connect_ap(void *arg, STATUS status)
{
	uint8 ssid[33];
    os_printf("wifi_station_scan done!!!\r\n");
    // Scan succeeded, proceed to connection
    if (status == OK) {
        struct bss_info *bss_link = (struct bss_info *)arg;
        // bss_link is a singly linked tail queue of bss_link structs
        bss_link = bss_link->next.stqe_next;
        // Print all wifi found in scan
        while (bss_link != NULL)
        {
            os_memset(ssid, 0, 33); // One more byte for NULL terminator
            
            // Read first 32 bit of SSID
            if (os_strlen(bss_link->ssid) <= 32)
            {
                os_memcpy(ssid, bss_link->ssid, os_strlen(bss_link->ssid)+1);
            }
            else
            {
                os_memcpy(ssid, bss_link->ssid, 32);
                os_printf("Detected SSID longer than 32 chars\r\n");
            }
            // Print detected AP infos to serial port
            os_printf("+CWLAP:(%d, \"%s\", %d, \""MACSTR"\", %d)\r\n",
                      bss_link->authmode, ssid, bss_link->rssi,
                      MAC2STR(bss_link->bssid), bss_link->channel);
            /* authmode: Autherization Mode    ssid: Primary ID of WLAN
             * rssi: Signal Strength    bssid: MAC address of AP
             * channel: AP's signal Channel (14 possible channels in total) */

            bss_link = bss_link->next.stqe_next;
        }

        // Connect to client AP station
        struct station_config stationConf;
        // Clear struct to prevent garbage values, then set SSID and password
        os_memset(&stationConf, 0, sizeof(struct station_config));
        os_memcpy(&stationConf.ssid, AP_SSID, os_strlen(AP_SSID)+1);
        os_memcpy(&stationConf.password, AP_PASSWD, os_strlen(AP_PASSWD)+1);
        // Store AP SSID and password in ram (settings in flash unchanged)
        wifi_station_set_config_current(&stationConf);
        // Attempt connection
        wifi_station_connect();

        // Check if AP is connected every timer interval with software timer
        os_timer_disarm(&ap_connect_timer);
        os_timer_setfn(&ap_connect_timer, 
                       (os_timer_func_t *)check_ap_connected, NULL);
        os_timer_arm(&ap_connect_timer, 3000, 0);

    // Scan failed, retry after delay
    } else {
        os_printf("WiFi scan unsuccessful... Retrying...\r\n");
        os_delay_ms(10);
        wifi_station_scan(NULL, connect_ap);
    }
}

/*
 * Timer callback function to check AP connection status
 */
void ICACHE_FLASH_ATTR
check_ap_connected(void *arg) 
{
    os_timer_disarm(&ap_connect_timer);
    // Retreive current connection status
	static uint8 check_count = 0;
    static uint8 fail_count = 0;
	uint8 status;
	check_count++;
	status = wifi_station_get_connect_status();
	os_printf("Connection status is %u at check %u\r\n", status, check_count);

	struct ip_info ipconfig;
	wifi_get_ip_info(STATION_IF, &ipconfig); // retreive local ip address

    // Connetion successful
	if (status == STATION_GOT_IP && ipconfig.ip.addr != 0)
    { 
        os_printf("Successful connection to access point!!!\r\n");
        // ################## Operation logic entry point ##################
        my_station_init((struct ip_addr *)remote_ip, &ipconfig.ip, remote_connect_port);
        // Proceed to initialize GPIO interface with TM4C
        user_GPIO_init();
        // Check input from TM4C every 500ms, send to server when input change
        os_timer_disarm(&gpio_check_timer);
        os_timer_setfn(&gpio_check_timer, 
                       (os_timer_func_t *)check_command, NULL);
        os_timer_arm(&gpio_check_timer, 500, 0);
	}
    // Connection failed
    else if (status == STATION_WRONG_PASSWORD
          || status == STATION_NO_AP_FOUND
          || status == STATION_CONNECT_FAIL)
    {
        os_printf("Connection failed with %u check(s)", check_count);
        fail_count++;
        if (fail_count >= 10)
        { 
            // Connections failed too many times. Check SSID or password.
            os_printf("Connection failed or timed out 10 times！Giving up\r\n");
            fail_count = 0;
            return;
        }
        else
        {
            // Attempt reconnection
            wifi_station_scan(NULL, connect_ap);
        }
    }
    // Still connecting (or local ip address is 0), re-arm timer
    else
    {
        // Toke longer than 18 seconds to connect
        if (check_count >= 150)
        {
            os_printf("Connection time out!\r\n");
            fail_count++;
            check_count = 0;
            // Attempt reconnection
            wifi_station_scan(NULL, connect_ap);
        }
        // Still waiting, check every 0.1 seconds from now on until timeout
        else
        {
            os_timer_setfn(&ap_connect_timer,
                           (os_timer_func_t *)check_ap_connected, NULL);
            os_timer_arm(&ap_connect_timer, 100, 0);
        }
    }
}

/*
 * Configure GPIO 0 and 2 to input mode
 */
void ICACHE_FLASH_ATTR
user_GPIO_init(void)
{
    // Enable GPIO 0 and 2
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    // Disable output
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(0));
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));
    // Disable pullup resisters
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);
}

/*
 * Check GPIO, do HTTP post request to server when input command changed
 */
void ICACHE_FLASH_ATTR
check_command(void)
{
    // Send post request to server if input command changed
    last_command = current_command;
    current_command = GPIO_INPUT_GET(GPIO_ID_PIN(0)) & 
                     (GPIO_INPUT_GET(GPIO_ID_PIN(2)) << 1);
    if (current_command != last_command && current_command != 0)
    {
        // Start connection
	    espconn_connect(&user_tcp_conn);
    }
    // Setup timer for next check
    os_timer_setfn(&gpio_check_timer, 
                   (os_timer_func_t *)check_command, NULL);
    os_timer_arm(&gpio_check_timer, 500, 0);
}

/*
 * Monitors WiFi state
 */
void ICACHE_FLASH_ATTR
wifi_event_handler_cb(System_Event_t *evt)
{
    if (evt == NULL)
    {
        os_printf("WiFi event handler callback triggered with NULL event\r\n");
        return;
    }

    switch (evt->event)
    {
        case EVENT_STAMODE_CONNECTED:
            os_printf("Connected to ssid %s, channel %d\n",
                      evt->event_info.connected.ssid,
                      evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            os_printf("Disconnected from ssid %s, reason %d\r\n", 
                      evt->event_info.disconnected.ssid,
                      evt->event_info.disconnected.reason);
            os_printf("(refer to REASON enum in user_interface.h)\r\n");
            // Attempt reconnection
            wifi_station_scan(NULL, connect_ap);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            os_printf("authmode change: %d -> %d\r\n",
                      evt->event_info.auth_change.old_mode,
                      evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
                      IP2STR(&evt->event_info.got_ip.ip),
                      IP2STR(&evt->event_info.got_ip.mask),
                      IP2STR(&evt->event_info.got_ip.gw));
            os_printf("\r\n");
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            // Other device connected
            os_printf("station: " MACSTR "joined, AID = %d\r\n",
                      MAC2STR(evt->event_info.sta_connected.mac),
                      evt->event_info.sta_connected.aid);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            // Other device disconnected
            os_printf("station: " MACSTR "left, AID = %d\r\n",
                      MAC2STR(evt->event_info.sta_disconnected.mac),
                      evt->event_info.sta_disconnected.aid);
            break;
        default:
            break;
    }
}

/*
 * Delay function in miliseconds (May be longer than expected)
 * SDK documentation recommends no longer than 15ms delay
 */
void os_delay_ms(uint16 ms)
{
    int i = 0;
    for (i = 0; i < ms; i++)
        os_delay_us(1000);
}