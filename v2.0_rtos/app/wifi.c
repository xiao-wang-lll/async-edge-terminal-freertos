#include "esp_at.h"
#include "console.h"
#include "app.h"
#include "page.h"
#include "weather.h"
#include "real_time.h"
#include "FREERTOS.h"
#include "task.h"

#define LOG_TAG "wifi"
#define LOG_LVL ELOG_LVL_INFO
#include "elog.h"


void wifi_init(void){


	log_i("[SYS] startup\n");//printf用不了因为USART1的tx是用来输出到esp的rx了，而USART2没初始化
	
	if(!esp_at_init())//Wifi1
	{
		log_e("[AT] init failed\n");	
		goto err;
	}
	log_i("[AT] inited\n");
	
	if(!esp_at_wifi_init())//Wifi2
	{
		log_e("[WIFI] init failed\n");	
		goto err;
	}
	log_i("[WIFI] inited\n");
	
	
	
	if (!esp_at_sntp_init())
	{
			log_e("[SNTP] init failed\n");
			goto err;
	}
	log_i("[SNTP] inited\n");
	return;
	
err:
	error_page_display("[WIFI] init failed");
	while(1){
				log_e("AT ERROR");
		}
}


//建立超时的wifi等待连接
void wifi_wait_connect(void){
	
		//Wifi3
		esp_at_connect_wifi(WIFI_SSID,WIFI_PASSWD,NULL);
		log_i("[WIFI] connecting\n");
	
		esp_wifi_info_t wifi;

		for(int i=0;i<10*1000;i++){
			
			if(esp_at_get_wifi_info(&wifi) && wifi_is_connected()){
				log_i("[AT] wifi info get success\n");	
				log_i("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d\n",
                wifi.ssid, wifi.bssid, wifi.channel, wifi.rssi);
				return;
			}
			vTaskDelay(pdMS_TO_TICKS(3000));
			
			// 【核心解药】
			// 如果最初复位后没连上(State 4)，且热点没有打开，ESP32可能躺平了，那之后就算打开热点也不会再次重连了。
			//区别于中途的断线重连，这个是复位的首次连接
			// 每隔 一段时间，再次发送连接指令，踹它起来干活！
			if(i > 0 && i % 3 == 0){
					log_i("[WIFI] State is 4 (Failed), Retrying AT+CWJAP...\n");
					log_i("[WIFI] again connecting\n");
					// 重新激活连接任务！
					esp_at_connect_wifi(WIFI_SSID, WIFI_PASSWD, NULL);
					
			}
		
		}
		
		log_e("WIFI connect error");
		error_page_display("[WIFI] connect error");
		
		

}
























