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
    wifi_set_opmode(SOFTAP_MODE); // set mode to softAP mode
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

    // Initialize GPIO connection to TM4C
    user_GPIO_init();
    /* Establish a TCP server for http(with JSON) POST or GET command
       to communicate with sender device. */
    user_webserver_init(SERVER_PORT);
}

/*
 * Configure GPIO 0 and 2
 */
void ICACHE_FLASH_ATTR
user_GPIO_init(void)
{

    return;
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
