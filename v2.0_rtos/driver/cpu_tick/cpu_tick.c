
#include <stdint.h>
#include "stm32f4xx.h"
#include "cpu_tick.h"
#include "rtc.h"

#define TICKS_PER_MS SystemCoreClock/1000
#define TICKS_PER_US SystemCoreClock/1000/1000

uint64_t volatile cpu_tick_count=0;
static cpu_periodic_callback_t periodic_callback;

void cpu_tick_init(void){

		SysTick->LOAD=SystemCoreClock/1000;  //这里因为处理ms，所以要知道：CPU主频168MHz，1us对应168，所以1ms对应168/1000
		SysTick->VAL=0;
		SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}



uint64_t cpu_now(void){
	
	uint64_t temp_count=0;
	uint64_t res_count=0;
	do{
		temp_count =cpu_tick_count;
	
		//返回过了几ms下的+几us，比如过了1个ms又加3us---注意这里的时间先用tick的count表示
		//但是怕这个原子操作执行一半突然进中断，导致求和到加法右边操作的时候，前面的count在中断里面改变了
		//所以使用do while进行count判断
		res_count= cpu_tick_count + (SysTick->LOAD - SysTick->VAL);
	
	}while(temp_count != cpu_tick_count);
	
	return res_count;
}


uint64_t cpu_get_ms(void){
	
	//ms的count/(1ms走了多少count)= 多少ms
	return cpu_tick_count/TICKS_PER_MS;
}

uint64_t cpu_get_us(void){
	
	//us的count/(1us走了多少count)== 多少us
	return cpu_tick_count/TICKS_PER_US;
}


void cpu_delay_us(uint32_t us){
		
	uint64_t now=cpu_now();
	while(cpu_now() - now <= (uint64_t)us * TICKS_PER_US);

}


void cpu_delay_ms(uint32_t ms){
		
	uint64_t now=cpu_now();
	while(cpu_now() - now <= (uint64_t)ms * TICKS_PER_MS);

}



/****************/
void cpu_register_periodic_callback(cpu_periodic_callback_t func){

		periodic_callback=func;
}




//让count自动实现每1ms 下的tick++(因为前面init里面初始化1ms触发一次中断)，主要是后面为了获取当前过了几ms，只不过这里时间都以count表示
//[新增定时渲染功能]  让时间每隔
void SysTick_Handler(void){
	
	cpu_tick_count+=TICKS_PER_MS;
	
	
	//执行回调函数
	if(periodic_callback){
		periodic_callback();
	}
	
	
	

}


