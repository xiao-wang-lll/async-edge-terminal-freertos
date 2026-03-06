#include "stm32f4xx.h"
#include "st7789.h"
#include "cpu_tick.h"
#include "console.h"
#include "stdio.h"
#include "rtc.h"
#include "app.h"
#include "esp_at.h"
#include "aht20.h"

void board__lowlevel_init(void){
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//stm32串口操作esp32
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);//stm32串口调试printf
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//stm32串口调试printf
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	

}


void board_init(void){
	
	cpu_tick_init();
	st7789_init();
	console_init();//stm32的串口2的初始化，即printf调试日志，区别于串口1发送esp指令
	printf("[SYS] Build Date：%s %s\r\n",__DATE__,__TIME__);
	
	//aht20_init();
	rtc_init();
	main_loop_init();

}



int fputc(int c,FILE *f){
	
	USART_ClearFlag(USART2,USART_FLAG_TXE);

	USART_SendData(USART2,(uint8_t)c);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
	
	return c;


}


