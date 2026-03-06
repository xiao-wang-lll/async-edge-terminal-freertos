#include "rtc.h"
#include "stm32f4xx.h"
#include "string.h"

void rtc_init(void){

	PWR_BackupAccessCmd(ENABLE);
	RCC_LSEConfig(RCC_LSE_ON);
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET){}
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
	RTC_InitTypeDef RTC_InitStructure;
	RTC_StructInit(&RTC_InitStructure);
	RTC_Init(&RTC_InitStructure);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();
  /* Enable The TimeStamp */
  RTC_TimeStampCmd(RTC_TimeStampEdge_Falling, ENABLE);    


}



void rtc_get_time_single(rtc_date_time_t *date_time){

	RTC_DateTypeDef RTC_DateStructure;
	RTC_TimeTypeDef RTC_TimeStructure;

	RTC_DateStructInit(&RTC_DateStructure);
	RTC_TimeStructInit(&RTC_TimeStructure);
	
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

	
	date_time->year=RTC_DateStructure.RTC_Year + 2000;
	date_time->month=RTC_DateStructure.RTC_Month;
	date_time->day=RTC_DateStructure.RTC_Date;
	date_time->hour=RTC_TimeStructure.RTC_Hours;
	date_time->minute=RTC_TimeStructure.RTC_Minutes;
	date_time->second=RTC_TimeStructure.RTC_Seconds;
	
	date_time->weekday=RTC_DateStructure.RTC_WeekDay;


}

void rtc_get_time(rtc_date_time_t *date_time){
	
	rtc_date_time_t time1,time2;
	
	do{
	
		rtc_get_time_single(&time1);
		rtc_get_time_single(&time2);
	
	}while(memcmp(&time1,&time2,sizeof(rtc_date_time_t)) != 0);

	memcpy(date_time,&time1,sizeof(rtc_date_time_t));

}
	

void rtc_set_time_single(rtc_date_time_t *date_time){

	RTC_DateTypeDef RTC_DateStructure;
	RTC_TimeTypeDef RTC_TimeStructure;

	RTC_DateStructInit(&RTC_DateStructure);
	RTC_TimeStructInit(&RTC_TimeStructure);
	
	
	
	 RTC_DateStructure.RTC_Year =date_time->year  - 2000;
	 RTC_DateStructure.RTC_Month=date_time->month;
	 RTC_DateStructure.RTC_Date=date_time->day;   
	 RTC_TimeStructure.RTC_Hours=date_time->hour;  
	 RTC_TimeStructure.RTC_Minutes=date_time->minute;
	 RTC_TimeStructure.RTC_Seconds=date_time->second;
	
	 RTC_DateStructure.RTC_WeekDay=date_time->weekday;
	
	
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);



}

void rtc_set_time(rtc_date_time_t *date_time){
	
	rtc_date_time_t time1;
	
	do{
		rtc_set_time_single(date_time);
		rtc_get_time_single(&time1);
	
	}while(memcmp(&time1,date_time,sizeof(rtc_date_time_t)) != 0);

	memcpy(date_time,&time1,sizeof(rtc_date_time_t));

}
	
	
	
	
	