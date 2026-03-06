

#include "stdbool.h"
#include "stdint.h"
#include "real_time.h"
#include "string.h"
#include "stdio.h"

//响应数据月份转时间。比如jan-》1
static uint8_t month_str_to_num(const char *month_str)
{
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for (uint8_t i = 0; i < 12; i++)
	{
		if (strcmp(month_str, months[i]) == 0)
		{
			return i + 1;
		}
	}
	return 0;
}

//响应数据周转时间。比如Mon-》1
static uint8_t weekday_str_to_num(const char *weekday_str)
{
	const char *weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	for (uint8_t i = 0; i < 7; i++) {
		if (strcmp(weekday_str, weekdays[i]) == 0)
		{
			return i + 1;
		}
	}
	return 0;
}
//时间数据判断并解析
bool parse_cipsntptime_response(const char *response,esp_date_time_t *date){
	//	AT+CIPSNTPTIME?
	//	+CIPSNTPTIME:Sun Jul 27 14:07:19 2025
	//	OK
	char *start=strstr(response,"+CIPSNTPTIME:");
	if (start == NULL)
		return false;
	
	char weekday_str[8];
	char month_str[4];
	if(start){
		if(sscanf(start,"+CIPSNTPTIME:%3s %3s %hhu %hhu:%hhu:%hhu %hu", 
				  weekday_str, month_str, &date->day, &date->hour, &date->minute, &date->second, &date->year) != 7){
			return false;
		}
		date->weekday = weekday_str_to_num(weekday_str);
		date->month = month_str_to_num(month_str);		
	}	
	return true;
}
