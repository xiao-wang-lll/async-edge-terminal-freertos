
#include "esp_at.h"
#include "console.h"
#include "app.h"
#include "page.h"
#include "weather.h"
#include "real_time.h"







void wifi_init(void){


	printf("[SYS] startup\n");//printf用不了因为USART1的tx是用来输出到esp的rx了，而USART2没初始化
	
	if(!esp_at_init())//Wifi1
	{
		printf("[AT] init failed\n");	
		goto err;
	}
	printf("[AT] inited\n");
	
	if(!esp_at_wifi_init())//Wifi2
	{
		printf("[WIFI] init failed\n");	
		goto err;
	}
	printf("[WIFI] inited\n");
	
	
	
	if (!esp_at_sntp_init())
	{
			printf("[SNTP] init failed\n");
			goto err;
	}
	printf("[SNTP] inited\n");
	return;
	
err:
	error_page_display("[WIFI] init failed");
	while(1){
				printf("AT ERROR");
		}
}


//建立超时的wifi等待连接
void wifi_wait_connect(void){
	
		//Wifi3
		esp_at_connect_wifi(WIFI_SSID,WIFI_PASSWD,NULL);
		printf("[WIFI] connecting\n");
	
		esp_wifi_info_t wifi;

		for(int i=0;i<10*1000;i++){
			
			if(esp_at_get_wifi_info(&wifi) && wifi_is_connected()){
				printf("[AT] wifi info get success\n");	
				printf("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d\n",
                wifi.ssid, wifi.bssid, wifi.channel, wifi.rssi);
				return;
			}
			cpu_delay_ms(3000);
			
			// 【核心解药】
			// 如果最初复位后没连上(State 4)，且热点没有打开，ESP32可能躺平了，那之后就算打开热点也不会再次重连了。
			//区别于中途的断线重连，这个是复位的首次连接
			// 每隔 一段时间，再次发送连接指令，踹它起来干活！
			if(i > 0 && i % 3 == 0){
					printf("[WIFI] State is 4 (Failed), Retrying AT+CWJAP...\n");
					printf("[WIFI] again connecting\n");
					// 重新激活连接任务！
					esp_at_connect_wifi(WIFI_SSID, WIFI_PASSWD, NULL);
					
			}
		
		}
		
		printf("WIFI connect error");
		error_page_display("[WIFI] connect error");
		
		
		
		
		
//		//Wifi4---获取天气
//		esp_weather_info_t weather = { 0 };
//		const char *weather_http_response=esp_at_http_get(url);
//		if(weather_http_response == NULL){
//				printf("[Weather]http error\n");	
//				continue;
//		}
//		if(!parse_seniverse_response(weather_http_response,&weather))
//		{
//				printf("[WEAHTER] parse failed\n");
//        continue;
//		}
//		
//		printf("[WEATHER] %s, %s, %.1f\n", weather.city, weather.weather, weather.temperature);
//		
//		
//		
//		//esp_at_sntp_init();----获取时间
//		esp_date_time_t date={0};
//		if(!esp_at_sntp_get_time(&date))
//		{
//				printf("[SNTP] parse failed\n");
//        continue;
//		}
//		if (date.year > 2000)
//        {
//            printf("[SNTP] %04u-%02u-%02u %02u:%02u:%02u (%s)\n",
//                date.year, date.month, date.day, date.hour, date.minute, date.second,
//                date.weekday == 1 ? "Monday":
//                date.weekday == 2 ? "Tuesday":
//                date.weekday == 3 ? "Wednesday":
//                date.weekday == 4 ? "Thursday":
//                date.weekday == 5 ? "Friday":
//                date.weekday == 6 ? "Saturday":
//                date.weekday == 7 ? "Sunday" : "Unknown");
//        }
//	}
}
























