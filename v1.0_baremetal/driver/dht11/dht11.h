#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f4xx.h"                  // Device header



#define dht11_high GPIO_SetBits(GPIOD, GPIO_Pin_14)
#define dht11_low GPIO_ResetBits(GPIOD, GPIO_Pin_14)
#define Read_Data GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_14)

void DH11_GPIO_Init_OUT(void);
void DH11_GPIO_Init_IN(void);
void DHT11_Start(void);
unsigned char DHT11_REC_Byte(void);
//void DHT11_REC_Data(void);
void DHT11_REC_Data(float *temperature,float *humidity);

void dht11_simple(void);


#endif

