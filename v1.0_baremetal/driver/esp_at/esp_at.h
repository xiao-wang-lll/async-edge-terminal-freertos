#ifndef __ESP_AT_H
#define __ESP_AT_H

#include "stdint.h"
#include "stdbool.h"
#include "real_time.h"



#include "stm32f4xx.h"
#include "stdbool.h"
#include "string.h"
#include "esp_at.h"
#include "stdio.h"
#include "cpu_tick.h"
#include "console.h"

#define LED1_PORT GPIOA
#define LED2_PORT GPIOA
#define LED1_PIN GPIO_Pin_6
#define LED2_PIN GPIO_Pin_7



//制定响应Wifi回显数据的结构体
typedef struct{
	
	char ssid[64];
	char bssid[18];
	int channel;
	int rssi;
	bool connected;  //值为true表示connect+ip get
	
}esp_wifi_info_t;




void esp_at_usart_init(void);
bool esp_at_init(void);
void usart1_clear_rx_buffer(void);
bool esp_at_wait_ready(uint64_t timeout);
bool esp_at_write_command(char *command,uint64_t timeout);


const char *esp_at_get_response(void);
bool esp_at_wifi_init(void);
bool esp_at_connect_wifi(const char *ssid,const char *pwd,const char *mac);
bool esp_at_get_wifi_info(esp_wifi_info_t *info);
bool wifi_is_connected(void);
const char *esp_at_http_get(const char *url);

bool esp_at_sntp_init(void);
bool esp_at_sntp_get_time(esp_date_time_t *date);



#endif
