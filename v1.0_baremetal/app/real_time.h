#ifndef __REAL__TIME_AT_H
#define __REAL__TIME_AT_H

#include "stdbool.h"
#include "stdint.h"

//制定自定义的用来解析回显数据即时间然后储存下时间的结构体
typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t weekday;
} esp_date_time_t;


static uint8_t month_str_to_num(const char *month_str);
static uint8_t weekday_str_to_num(const char *weekday_str);
bool parse_cipsntptime_response(const char *response,esp_date_time_t *date);

#endif
