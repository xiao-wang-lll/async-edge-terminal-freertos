#ifndef __WEATHER_AT_H
#define __WEATHER_AT_H

#include "stdbool.h"
//制定响应HTTP回显数据的结构体
typedef struct{
	
	char city[32];
	char location[32];
	char weather[16];
	int weather_code;
	float temperature;
	
}esp_weather_info_t;

bool parse_seniverse_response(const char *response,esp_weather_info_t *info);

#endif
