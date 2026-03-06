#include <stdint.h>
#include "stdio.h"
#include "page.h"
#include "app.h"
#include "board.h"
#include "esp_at.h"
#include "string.h"
#include "aht20.h"
#include "weather.h"
#include "dht11.h"
#include "FREERTOS.h"
#include "task.h"
#include "timers.h"
#include "workqueue.h"

#define LOG_TAG "app"
#define LOG_LVL ELOG_LVL_INFO
#include <elog.h>

#define MILLISECONDS(x) (x)
#define SECONDS(x)      MILLISECONDS((x) * 1000)
#define MINUTES(x)      SECONDS((x) * 60)
#define HOURS(x)        MINUTES((x) * 60)
#define DAYS(x)          HOURS((x) * 24)

#define TIME_SYNC_INTERVAL          HOURS(1) //这种对于freertos的tick间隔太长了，缩减至1hour
#define WIFI_UPDATE_INTERVAL        SECONDS(5)
#define TIME_UPDATE_INTERVAL        SECONDS(1)
#define INNER_UPDATE_INTERVAL       SECONDS(3)
#define OUTDOOR_UPDATE_INTERVAL     MINUTES(3)


static TimerHandle_t time_sync_timer;
static TimerHandle_t wifi_update_timer;
static TimerHandle_t time_update_timer;
static TimerHandle_t inner_update_timer;
static TimerHandle_t outdoor_update_timer;



//time_sync_delay  时间同步方法设置即每隔固定时间动态刷新数据---因为rtc的时间不准
static void time_sync(void){

		uint32_t restart_sync_delay = TIME_SYNC_INTERVAL;
		
		rtc_date_time_t rtc_date={0};
		
		//1.先获取网络时间---时间保存在date结构体
			
		//esp_at_sntp_init();----SNTP配置
		//get时间
		esp_date_time_t esp_date={0};
		if(!esp_at_sntp_get_time(&esp_date))
		{
				log_e("[SNTP] get time failed");
				restart_sync_delay = SECONDS(1);
				goto err;
			
    }
		if (esp_date.year < 2000)
		{
				log_e("[SNTP] invalid date formate");
				restart_sync_delay = SECONDS(1);
				goto err;
		}
		log_i("[SNTP] sync time: %04u-%02u-%02u %02u:%02u:%02u (%d)",
        esp_date.year, esp_date.month, esp_date.day,
        esp_date.hour, esp_date.minute, esp_date.second, esp_date.weekday);		
		
		
		
		//2.将得到的时间显示在屏幕上
    rtc_date.year = esp_date.year;
    rtc_date.month = esp_date.month;
    rtc_date.day = esp_date.day;
    rtc_date.hour = esp_date.hour;
    rtc_date.minute = esp_date.minute;
    rtc_date.second = esp_date.second;
    rtc_date.weekday = esp_date.weekday;
    rtc_set_time(&rtc_date);
		
				
		//time_update_delay=10;  //这里专门设置这个去立马同步更新一次时间
		
//注意这里的err正常走也是会走下面的这句话		
err:
		//改变周期 等价于 重启定时器 和 重新开始超时时间，既而到时间了又回调发送task
		xTimerChangePeriod(time_sync_timer,pdMS_TO_TICKS(restart_sync_delay),0);
		
				
} 


//Wifi的更新---因为wifi强度等时常变化所以这里专注于connect
static void wifi_update(void){
	
		static esp_wifi_info_t last_info={0};//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应
		//改变周期 等价于 重启定时器 和 重新开始超时时间，既而到时间了又回调发送task
		xTimerChangePeriod(wifi_update_timer,pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL),0);
		

		esp_wifi_info_t info={0};
		if(!esp_at_get_wifi_info(&info)){
				log_e("[AT] wifi info get failed");
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
        log_i("[WIFI] connected to %s", info.ssid);
        log_i("[WIFI] SSID: %s, BSSID: %s, Channel: %d, RSSI: %d",
                info.ssid, info.bssid, info.channel, info.rssi);
        main_page_redraw_wifi_ssid(info.ssid);
    }
    else
    {
        log_w("[WIFI] disconnected from %s", last_info.ssid);
        main_page_redraw_wifi_ssid("wifi lost");
    }
    
    memcpy(&last_info, &info, sizeof(esp_wifi_info_t));

		
		
}

//时间+日期的更新
static void time_update(void){
	
		static rtc_date_time_t last_date={0};//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应
		//改变周期 等价于 重启定时器 和 重新开始超时时间，既而到时间了又回调发送update time
		xTimerChangePeriod(time_update_timer,pdMS_TO_TICKS(TIME_UPDATE_INTERVAL),0);
		
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

		//改变周期 等价于 重启定时器 和 重新开始超时时间，既而到时间了又回调发送task
			xTimerChangePeriod(inner_update_timer,pdMS_TO_TICKS(INNER_UPDATE_INTERVAL),0);
		
		
//		if (!aht20_start_measurement())
//    {
//        printf("[AHT20] start measurement failed\n");
//        return;
//    }
//    
//    if (!aht20_wait_for_measurement())
//    {
//        printf("[AHT20] wait for measurement failed");
//        return;
//    }
//    
//		
//		
//    float temperature = 0.0f, humidity = 0.0f;
//    
//    if (!aht20_read_measurement(&temperature, &humidity))
//    {
//        printf("[AHT20] read measurement failed");
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
	
	
		//改变周期 等价于 重启定时器 和 重新开始超时时间，既而到时间了又回调发送task
		xTimerChangePeriod(inner_update_timer,pdMS_TO_TICKS(INNER_UPDATE_INTERVAL),0);
	
		static float last_temperature, last_humidity;//这里很重要就是用来接收上一次的info信息，因为这里使用的static 此句对应

		
    float temperature = 0.0f, humidity = 0.0f;
    
	
    DHT11_REC_Data(&temperature,&humidity);
    
    if (temperature == last_temperature && humidity == last_humidity)
    {
        return;
    }
		
		last_temperature = temperature;
    last_humidity = humidity;
    
		log_i("[DHT11] Temperature: %.1f, Humidity: %.1f", temperature, humidity);
		main_page_redraw_inner_temperature(temperature);
		main_page_redraw_inner_humidity(humidity);
			

		
}







//户外天气的更新
static void outdoor_update(void){
	
		static esp_weather_info_t last_weather = { 0 };
		//改变周期 等价于 重启定时器 和 重新开始超时时间，既而到时间了又回调发送task
		xTimerChangePeriod(outdoor_update_timer,pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL),0);

		
		esp_weather_info_t  weather= { 0 };    	
    const char *weather_http_response = esp_at_http_get(WEATHER_URL);
		if(weather_http_response == NULL ){
			
			log_e("[WEATHER] http error");
			return;
		}
		
		if (!parse_seniverse_response(weather_http_response, &weather))
    {
        log_e("[WEATHER] parse failed");
        return;
    }
		
		
		if(memcmp(&weather,&last_weather,sizeof(esp_weather_info_t)) == 0){
				return;
		}
		
	
    
		
		memcpy(&last_weather, &weather, sizeof(esp_weather_info_t));
		log_i("[WEATHER] %s, %s, %.1f", weather.city, weather.weather, weather.temperature);

		main_page_redraw_outdoor_temperature(weather.temperature);
		main_page_redraw_outdoor_weather_icon(weather.weather_code);
		
		
		

}



typedef void (*app_job_t)(void);//函数指针去实现多个功能模块的执行


static void app_work(void *param){//工作队列拿到app_work函数名和event的参数去放入队列，然后拿出执行回调就是执行了这个函数
	
	app_job_t job=(app_job_t)param;
	job();
	
}



//workqueue工作队列下的回调执行各个模块---以队列方式执行
//因为这里多个模块的执行，需要进队列保持同步，防止冲突
void work_timer_cb(TimerHandle_t timer){

	uint32_t event = (uint32_t)pvTimerGetTimerID(timer);
	workqueue_run(app_work,(void *)event);//因为app_job_t指向的函数是无参的，不匹配这里的run里面的work_t的参数，需要另外开一个函数
	

}

//app本身的模块执行.因为下面的app这里的时间更新就一个函数所以不需要进队列
static void app_timer_cb(TimerHandle_t timer)
{
    app_job_t job=(app_job_t)pvTimerGetTimerID(timer);
		job();
}


//设置了定时器，创建任务然后发送全1去初始化催动所有功能
//所有功能里面各自有xTimerChangePeriod去启动 下方的定时器并之后不断的改变超时时间
void app_init(void)
{
    
	
	time_sync_timer = xTimerCreate("time sync", pdMS_TO_TICKS(TIME_SYNC_INTERVAL), pdFALSE, time_sync, work_timer_cb);
	wifi_update_timer = xTimerCreate("wifi update", pdMS_TO_TICKS(WIFI_UPDATE_INTERVAL), pdFALSE, wifi_update, work_timer_cb);
	time_update_timer = xTimerCreate("time update", pdMS_TO_TICKS(TIME_UPDATE_INTERVAL), pdFALSE, time_update, app_timer_cb);
	inner_update_timer = xTimerCreate("inner upadte", pdMS_TO_TICKS(INNER_UPDATE_INTERVAL), pdFALSE, inner_update_dht11, work_timer_cb);
	outdoor_update_timer = xTimerCreate("outdoor update", pdMS_TO_TICKS(OUTDOOR_UPDATE_INTERVAL), pdFALSE, outdoor_update, work_timer_cb);
			
	
	//先启动一次所有功能模块，同时因为功能模块里面的period的修改表示定时器也开始了
	workqueue_run(app_work,time_sync);
	workqueue_run(app_work,wifi_update);
	workqueue_run(app_work,time_update);
	workqueue_run(app_work,inner_update_dht11);
	workqueue_run(app_work,outdoor_update);
	
	

	
	
}




