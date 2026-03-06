
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "weather.h"

////state响应数据
//static const char *swstate_response="AT+CWSTATE?\r\n"
//"+CWSTATE:2,\"vivo\"\r\n"
//"\r\n"
//"OK\r\n";

////AP响应数据
//static const char *swjap_response="AT+CWJAP?\r\n"
//"+CWJAP:\"vivo\",\"4e:99:2c:35:b1:4d\",4,-53,0,1,3,0,1\r\n"
//"\r\n"
//"OK\r\n";


////weather响应数据
//static const char *seniverse_response="AT+HTTPCLIENT=2,0,\"https://api.seniverse.com/v3/weather/now.json?key=S75RkjmBtcG-hHVnn&location=beijing&language=en&unit=c\",,,2\r\n"
//"+HTTPCLIENT:259,{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"Beijing\",\"country\":\"CN\",\"path\":\"Beijing,Beijing,China\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"now\":{\"text\":\"Sunny\",\"code\":\"0\",\"temperature\":\"1\"},\"last_update\":\"2026-01-22T14:07:23+08:00\"}]}\r\n"
//"\r\n"
//"OK\r\n";






//http下的Weather回显解析
bool parse_seniverse_response(const char *response,esp_weather_info_t *info){
	//{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"Beijing\",\"country\":\"CN\",\"path\":\"Beijing,Beijing,China\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"now\":{\"text\":\"Sunny\",\"code\":\"0\",\"temperature\":\"1\"},\"last_update\":\"2026-01-22T14:07:23+08:00\"}]}\r\n"
	char *results=strstr(response,"\"results\"");
	if (results == NULL)
		return false;
	
	char *location=strstr(response,"\"location\"");
	if (location == NULL)
		return false;
	
	char *name=strstr(response,"\"name\"");
	if(name){
		if(sscanf(name, "\"name\":\"%31[^\"]\"", info->city)!=1){
			return false;
		}
	}
	
	char *path=strstr(response,"\"path\"");
	if(path){
		if(sscanf(path, "\"path\":\"%31[^\"]\"", info->location)!=1){
			return false;
		}
	}
	
	char *now=strstr(response,"\"now\"");
	if (now == NULL)
		return false;
	
	char *text=strstr(response,"\"text\"");
	if(text){
		if(sscanf(text, "\"text\":\"%15[^\"]\"", info->weather)!=1){
			return false;
		}
	}
	
	char *code=strstr(response,"\"code\"");
	if(code){
		if(sscanf(code, "\"code\":\"%d\"", &info->weather_code)!=1){
			return false;
		}
		
	}

	
	char *temperature=strstr(response,"\"temperature\"");
	if(temperature){
		char temp[16];
		if(sscanf(temperature, "\"temperature\":\"%15[^\"]\"", temp)!=1){
			return false;
		}
		info->temperature=atof(temp);
	}
	
	return true;
}






