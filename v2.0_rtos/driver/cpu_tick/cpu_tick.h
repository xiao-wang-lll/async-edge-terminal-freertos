#ifndef __CPU_TICK_H
#define __CPU_TICK_H

#include <stdint.h> 

typedef void (*cpu_periodic_callback_t)(void);


void cpu_tick_init(void);
uint64_t cpu_now(void);
uint64_t cpu_get_ms(void);
uint64_t cpu_get_us(void);
void cpu_delay_us(uint32_t us);
void cpu_delay_ms(uint32_t ms);
void SysTick_Handler(void);

void cpu_register_periodic_callback(cpu_periodic_callback_t func);

#endif


