#include <stdint.h>
#include "stdio.h"
#include "page.h"
#include "app.h"
#include "cpu_tick.h"
#include "board.h"
#include "esp_at.h"
#include "string.h"
#include "aht20.h"
#include "weather.h"
#include "app.h"
#include "dht11.h"

#define MILLISECONDS(x) (x)
#define SECONDS(x)      MILLISECONDS((x) * 1000)
#define MINUTES(x)      SECONDS((x) * 60)
#define HOURS(x)        MINUTES((x) * 60)
#define DAYS(x)          HOURS((x) * 24)

#define TIME_SYNC_INTERVAL          DAYS(1)
#define WIFI_UPDATE_INTERVAL        SECONDS(5)
#define TIME_UPDATE_INTERVAL        SECONDS(1)
#define INNER_UPDATE_INTERVAL       SECONDS(3)
#define OUTDOOR_UPDATE_INTERVAL     MINUTES(1)



static uint32_t time_sync_delay;        //时间设置 一天去由esp32去同步一次
static uint32_t wifi_update_delay;			//wifi设置5s去更新一次
static uint32_t time_update_delay;      //time设置1s去更新一次
static uint32_t inner_update_delay;     //室内温湿度设置3s去更新一次
static uint32_t outdoor_update_delay;   //户外天气设置1min去更新一次



//定义一个传入 注册回调函数的参数 的函数---之后回调函数执行的就是这个函数
static void cpu_periodic_callback(void){

		if (time_sync_delay > 0)
        time_sync_delay--;
    if (wifi_update_delay > 0)
        wifi_update_delay--;
    if (time_update_delay > 0)
        time_update_delay--;
    if (inner_update_delay > 0)
        inner_update_delay--;
    if (outdoor_update_delay > 0)
        outdoor_update_delay--;

} 


//注册回调函数
void main_loop_init(void)
{
    cpu_register_periodic_callback(cpu_periodic_callback);
}



//time_sync_delay  时间同步方法设置即每隔固定时间动态刷新数据---因为rtc的时间不准
static void time_sync(void){

		if(time_sync_delay != 0){  //如果不是0表示还没到时间间隔
			return;
		
		}
		
		//如果归0了，就先赋值回去，让他之后按这个间隔去--
		time_sync_delay = TIME_SYNC_INTERVAL;
		
		
		
		
		
		//1.先获取网络时间---时间保存在date结构体
			
		//esp_at_sntp_init();----SNTP配置
		//get时间
		esp_date_time_t esp_date={0};
		if(!esp_at_sntp_get_time(&esp_date))
		{
				printf("[SNTP] get time failed\n");
				time_sync_delay = SECONDS(5);
				return;
			
    }
		if (esp_date.year < 2000)
		{
				printf("[SNTP] invalid date formate\n");
				time_sync_delay = SECONDS(5);
				return;
		}
		printf("[SNTP] sync time: %04u-%02u-%02u %02u:%02u:%02u (%d)\n",
        esp_date.year, esp_date.month, esp_date.day,
        esp_date.hour, esp_date.minute, esp_date.second, esp_date.weekday);		
		
		
		//2.将得到的时间显示在屏幕上
		rtc_date_time_t rtc_date={0};
		
    rtc_date.year = esp_date.year;
    rtc_date.month = esp_date.month;
    rtc_date.day = esp_date.day;
    rtc_date.hour = esp_date.hour;
    rtc_date.minute = esp_date.minute;
    rtc_date.second = esp_date.second;
    rtc_date.weekday = esp_date.weekday;
    rtc_set_time(&rtc_date);
		
				
		time_update_delay=10;  //这里专门设置这个去防止多任务时造成
				
} 


//Wifi的更新---因为wifi强度等时常变化所以这里专注于connect
static void wifi_update(void){
	
		static esp_wifi_info_t last_info={0};//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应

		if(wifi_update_delay != 0){  //如果不是0表示还没到时间间隔
			return;
		
		}
		
		//如果归0了，就先赋值回去，让他之后按这个间隔去--
		wifi_update_delay = WIFI_UPDATE_INTERVAL;
		
		esp_wifi_info_t info={0};
		if(!esp_at_get_wifi_info(&info)){
				printf("[AT] wifi info get failed\n");
				return;
		
		}
		
		if(memcmp(&info,&last_info,sizeof(esp_wifi_info_t)) == 0){
				return;
		}
		
		//如果二者不同，但是连接状态一样就返回
		if (last_info.connected == info.connected)
    {
        return;
    }
		
		 if (info.connected)
    {
        printf("[WIFI] connected to %s\n", info.ssid);
        printf("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d\n",
                info.ssid, info.bssid, info.channel, info.rssi);
        main_page_redraw_wifi_ssid(info.ssid);
    }
    else
    {
        printf("[WIFI] disconnected from %s\n", last_info.ssid);
        main_page_redraw_wifi_ssid("wifi lost");
    }
    
    memcpy(&last_info, &info, sizeof(esp_wifi_info_t));


}

//时间+日期的更新
static void time_update(void){
	
		static rtc_date_time_t last_date={0};//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应

		if(time_update_delay != 0){  //如果不是0表示还没到时间间隔
			return;
		
		}
		
		//如果归0了，就先赋值回去，让他之后按这个间隔去--
		time_update_delay = TIME_UPDATE_INTERVAL;
		
		rtc_date_time_t date={0};
		rtc_get_time(&date);
		if(date.year < 2020){
				return;
		
		}
		
		if(memcmp(&date,&last_date,sizeof(rtc_date_time_t)) == 0){
				return;
		}
		
	
    
		main_page_redraw_date(&date);
		main_page_redraw_time(&date);

		memcpy(&last_date, &date, sizeof(rtc_date_time_t));

}


//室内温湿度的更新---使用aht20
static void inner_update(void){
	
		//static float last_temperature, last_humidity;//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应

		if(inner_update_delay != 0){  //如果不是0表示还没到时间间隔
			return;
		
		}
		
		//如果归0了，就先赋值回去，让他之后按这个间隔去--
		inner_update_delay = INNER_UPDATE_INTERVAL;
		
		
		
//		if (!aht20_start_measurement())
//    {
//        printf("[AHT20] start measurement failed\n");
//        return;
//    }
//    
//    if (!aht20_wait_for_measurement())
//    {
//        printf("[AHT20] wait for measurement failed\n");
//        return;
//    }
//    
//		
//		
//    float temperature = 0.0f, humidity = 0.0f;
//    
//    if (!aht20_read_measurement(&temperature, &humidity))
//    {
//        printf("[AHT20] read measurement failed\n");
//        return;
//    }
//    
//    if (temperature == last_temperature && humidity == last_humidity)
//    {
//        return;
//    }
//		
//		last_temperature = temperature;
//    last_humidity = humidity;
    
//		main_page_redraw_inner_temperature(temperature);
//		main_page_redraw_inner_humidity(humidity);
			
			
			main_page_redraw_inner_temperature(25.0);
			main_page_redraw_inner_humidity(50.0);	

}


//室内温湿度的更新---使用dht11
static void inner_update_dht11(void){
	
		static float last_temperature, last_humidity;//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应

		if(inner_update_delay != 0){  //如果不是0表示还没到时间间隔
			return;
		
		}
		
		//如果归0了，就先赋值回去，让他之后按这个间隔去--
		inner_update_delay = INNER_UPDATE_INTERVAL;
		

		
    float temperature = 0.0f, humidity = 0.0f;
    
    DHT11_REC_Data(&temperature,&humidity);
    
    if (temperature == last_temperature && humidity == last_humidity)
    {
        return;
    }
		
		last_temperature = temperature;
    last_humidity = humidity;
    
		main_page_redraw_inner_temperature(temperature);
		main_page_redraw_inner_humidity(humidity);
			

}












//户外天气的更新
static void outdoor_update(void){
	
		static esp_weather_info_t last_weather = { 0 };

		if(outdoor_update_delay != 0){  //如果不是0表示还没到时间间隔
			return;
		
		}
		
		//如果归0了，就先赋值回去，让他之后按这个间隔去--
		outdoor_update_delay = OUTDOOR_UPDATE_INTERVAL;
		
		esp_weather_info_t  weather= { 0 };    	
    const char *weather_http_response = esp_at_http_get(WEATHER_URL);
		if(weather_http_response == NULL ){
			
			printf("[WEATHER] http error\n");
			return;
		}
		
		if (!parse_seniverse_response(weather_http_response, &weather))
    {
        printf("[WEATHER] parse failed\n");
        return;
    }
		
		
		if(memcmp(&weather,&last_weather,sizeof(esp_weather_info_t)) == 0){
				return;
		}
		
	
    
		
		memcpy(&last_weather, &weather, sizeof(esp_weather_info_t));
		printf("[WEATHER] %s, %s, %.1f\n", weather.city, weather.weather, weather.temperature);

		main_page_redraw_outdoor_temperature(weather.temperature);
		main_page_redraw_outdoor_weather_icon(weather.weather_code);

}


void main_loop(void)
{
    time_sync();
    wifi_update();
    time_update();
    //inner_update();//使用下面的dht11方式实现温湿度检测
		inner_update_dht11();
    outdoor_update();
}




