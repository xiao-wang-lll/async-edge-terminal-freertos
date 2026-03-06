
#include "stm32f4xx.h"
#include "stdbool.h"
#include "string.h"
#include "console.h"
#include "FREERTOS.h"
#include "semphr.h"



console_received_func_t console_func;
static SemaphoreHandle_t write_async_semaphore;


static void console_io_init(void)
{
    /* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void console_usart_init(void)
{
    USART_InitTypeDef USART_InitStructure;
		USART_StructInit(&USART_InitStructure);
	
		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

			
		
		/* USART configuration */
		USART_Init(USART2, &USART_InitStructure);
		
		USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
		USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);
			/* Enable USART */
		USART_Cmd(USART2, ENABLE);
}

static void console_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_Channel = DMA_Channel_4;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_INC8;
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream6, &DMA_InitStruct);
	
		//DMA开启中断
		DMA_ITConfig(DMA1_Stream6,DMA_IT_TC,ENABLE);
}

static void console_int_init(void)
{
    //NVIC初始化
		NVIC_InitTypeDef NVIC_InitStructure;
		memset(&NVIC_InitStructure,0,sizeof(NVIC_InitStructure));
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
		
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
		NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;

		NVIC_Init(&NVIC_InitStructure);
	
		NVIC_InitStructure.NVIC_IRQChannel=DMA1_Stream6_IRQn;
		NVIC_Init(&NVIC_InitStructure);
}

void console_init(void)
{
		write_async_semaphore = xSemaphoreCreateBinary();
    configASSERT(write_async_semaphore);
	
	
    console_usart_init();
    console_dma_init();
    console_int_init();
    console_io_init();
    
}


void console_write(const char c[],uint32_t size){
	
	  int len=size;
		
	do{
			uint32_t chunk_size = len < 65535 ? len : 65535;
		
			DMA1_Stream6->NDTR = chunk_size;
			DMA1_Stream6->M0AR = (uint32_t)c;
			DMA1_Stream6->CR |= DMA_SxCR_MINC; 
			
			DMA_Cmd(DMA1_Stream6,ENABLE);
			
			xSemaphoreTake(write_async_semaphore,portMAX_DELAY);
			DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6);
			
			c += chunk_size;
			len -= chunk_size;
    } while (len > 0);
		
		//保证发送数据都发完了
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET);
		USART_ClearFlag(USART2, USART_FLAG_TC);
	
}

void console_received_register(console_received_func_t func){

	console_func=func;


}

void USART2_IRQHandler(void){
	
	uint8_t data=0x00;
	
	if(USART_GetFlagStatus(USART2,USART_FLAG_RXNE) != RESET){
		
		data=USART_ReceiveData(USART2);
		USART_ClearFlag(USART2,USART_FLAG_RXNE);
		
		
		if(console_func !=NULL){
		  console_func(data);
	
		}else{
				
			USART_SendData(USART2,0x0000);
		}
	}

}

//在该中断实现DMA的TC完就触发中断，否则就原地信号量一直在等待
void DMA1_Stream6_IRQHandler(void){
	
	
	if(DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) == SET){
	
		BaseType_t pxHigherPriorityTaskWoken;
		xSemaphoreGiveFromISR(write_async_semaphore,&pxHigherPriorityTaskWoken);
		//切换任务	
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		
		DMA_ClearITPendingBit(DMA1_Stream6, DMA_FLAG_TCIF6);
		
	}
	
	
}


