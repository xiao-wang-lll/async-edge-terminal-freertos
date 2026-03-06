
#include "stm32f4xx.h"
#include "stdbool.h"
#include "string.h"
#include "esp_at.h"
#include "stdio.h"
#include "console.h"
#include "real_time.h"
#include "FREERTOS.h"
#include "task.h"
#include "semphr.h"


// 假设 ESP32 的 RST 引脚接在 STM32 的某个 GPIO 上（比如 PA4，请根据实际电路修改）
#define ESP_RST_PIN  GPIO_Pin_4 
#define ESP_RST_PORT GPIOA


//#define ESP_AT_DEBUG  0
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

#define LOG_TAG "esp-at"
#define LOG_LVL ELOG_LVL_INFO
#include "elog.h"




//定义esp回显的各个数据
typedef enum{

		AT_ACK_NONE,
    AT_ACK_OK,
    AT_ACK_ERROR,
    AT_ACK_BUSY,
    AT_ACK_READY
	
}at_ack_t;



//映射的时候需要这个对象，根据对象的名称去获取名称对应的具体回显值
//相当于key-value
typedef struct{

	at_ack_t ack;
	const char *string;	

}at_ack_match_t;



static const at_ack_match_t at_ack_matches[]={

	{AT_ACK_OK,"OK\r\n"},
	{AT_ACK_ERROR,"ERROR\r\n"},
	{AT_ACK_BUSY,"busy p…\r\n"},
	{AT_ACK_READY,"ready\r\n"},

};

static char arr[1024]={'\0'}; 
static SemaphoreHandle_t at_ack_sempahore;
static const char *line=arr;//const表示值不可改，地址可改,始终指向arr，用于每次接收数据判断是否匹配AT回显指令
static uint16_t arr_len=0;	//表示存储回显指令的长度
static at_ack_t rxack; //用于存储rx接收完成后真正回显的指令

static void esp_at_io_init(void)
{
    	/* Connect PXx to USARTx_Tx*/
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);

		/* Connect PXx to USARTx_Rx*/
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		
		/* Configure USART Tx as alternate function  */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* Configure USART Rx as alternate function  */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
}

static void esp_at_usart_init(void)
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
		USART_Init(USART1, &USART_InitStructure);
		
		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
		
			/* Enable USART */
		USART_Cmd(USART1, ENABLE);
}

static void esp_at_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    DMA_StructInit(&DMA_InitStruct);
	
    DMA_InitStruct.DMA_Channel = DMA_Channel_4;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_INC8;
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream7, &DMA_InitStruct);
}

static void esp_at_int_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_SetPriority(USART1_IRQn, 5);
}


static void esp_at_lowlevel_init(void)
{
    esp_at_usart_init();
    esp_at_dma_init();
    esp_at_int_init();
    esp_at_io_init();
}

bool esp_at_init(void)
{
	at_ack_sempahore = xSemaphoreCreateBinary();
	configASSERT(at_ack_sempahore);
	
	esp_hard_reset();
	esp_at_lowlevel_init();
	


	esp_at_write_command("AT\r\n",100);
	
	if (!esp_at_write_command("AT\r\n", 100)){
    return false;
	}
	//wifi---1
	if(!esp_at_write_command("AT+RESTORE\r\n",1000)){
		return false;
	}
	
	if(!esp_at_wait_ready(5000)){
		return false;
	}
	return true;
}


// 辅助函数：专门用来倒垃圾的
void usart1_clear_rx_buffer(void){
    // 只要串口还有数据（RXNE标志位是1），就读出来扔掉
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET){
        USART_ReceiveData(USART1); // 读出来，不保存，就是扔掉
    }
}



//映射函数---作用是把具体的指令映射成简化的enum值
static at_ack_t match_internal_ack(const char *str){
	
	for(uint8_t i=0;i<ARRAY_SIZE(at_ack_matches);i++){  //这里的固定值i<4,即枚举的内容数量，直接变为动态的
		//因为我的硬件原因只能换为strste，原视频为：strcmp(at_ack_matches[i].string,str)==0
		if(strstr(str,at_ack_matches[i].string) != NULL){
			return at_ack_matches[i].ack;
		
		}
	}

	return AT_ACK_NONE;

}





//发送命令函数，这里不用加dowhile循环
static void esp_at_usart_write(char *command){
	

		  int len=strlen(command);
			DMA2_Stream7->NDTR = len;
			DMA2_Stream7->M0AR = (uint32_t)command;
			DMA2_Stream7->CR |= DMA_SxCR_MINC; 
			
			
			DMA_ClearFlag(DMA2_Stream7,DMA_FLAG_TCIF7);
			DMA_Cmd(DMA2_Stream7,ENABLE);

		//保证发送数据都发完在接收部分实现

}

//接收esp回显的所有类型的指令
static at_ack_t esp_at_usart_wait_receive(uint64_t timeout){


		// 【自身 关键步骤 2】static 数组里的旧数据必须清零、或者采用视频里面的while下不断赋值为'\0'
		memset(arr,0,sizeof(arr));
		
		line = arr;
		arr_len = 0;
	
		BaseType_t x=xSemaphoreTake(at_ack_sempahore,pdMS_TO_TICKS(timeout));
		
		return x == pdPASS ? rxack : AT_ACK_NONE;

}

//判断esp的回显的ready的指令正确与否
bool esp_at_wait_ready(uint64_t timeout){

		return esp_at_usart_wait_receive(timeout) == AT_ACK_READY;

}


/**
主要就是使用stm32控制esp32，并且要真正捕获到esp回显的每个数据（OK、ready、ERROR、等等）

1.首先先发送AT指令
2.之后判断回应 成功 还是 失败
3.增加超时限制
4.为了分辨出接收到ready和busy...等回显数据，直接添加一个映射方法去获取对应的回显数据
5.然后封装发送指令函数和接收回显数据函数，以及增加和优化出判断OK与ready的回显数据正确与否函数

*/
bool esp_at_write_command(char *command,uint64_t timeout){
		// 【自身 关键步骤 1】发送前，先把之前的垃圾全部清空！
    // 这样确保后面 while 读到的，绝对是针对本次 command 的回复
    usart1_clear_rx_buffer();
	
	log_d("send: %s", command);
	
		esp_at_usart_write(command);
		at_ack_t ack=esp_at_usart_wait_receive(timeout);
		
	log_d("response: %s", rxbuf);
		
		
		return ack == AT_ACK_OK;
	
}


/*************************************************************************************/

//const---指针自由，数据锁住
const char *esp_at_get_response(void){

	return arr;

}

//wifi---2

bool esp_at_wifi_init(){

	return esp_at_write_command("AT+CWMODE=1\r\n",1000);

}




//Wifi下的satate回显解析
bool parse_cwstate_response(const char *response,esp_wifi_info_t *info){
	//"+CWSTATE:2,\"vivo\"\r\n"
	char *start=strstr(response,"+CWSTATE:");
	if (start == NULL)
		return false;
	
	int wifi_state=0;
	if(start){
		if(sscanf(start, "+CWSTATE:%d,\"%63[^\"]\"", &wifi_state, info->ssid)!=2){
			return false;
		}
		info->connected=(wifi_state == 2);
		
	}	
	return true;	
}
//Wifi下的AP回显解析
bool parse_swjap_response(const char *response,esp_wifi_info_t *info){
	//"+CWJAP:\"vivo\",\"4e:99:2c:35:b1:4d\",4,-53,0,1,3,0,1\r\n"
	char *start=strstr(response,"+CWJAP:");
	if (start == NULL)
		return false;
	
	
	if(start){
		if(sscanf(start, "+CWJAP:\"%63[^\"]\",\"%17[^\"]\",%d,%d", info->ssid,info->bssid,&info->channel,&info->rssi)!=4){
			return false;
		}
	}	
	return true;	
}





//wifi---3
bool esp_at_connect_wifi(const char *ssid,const char *pwd,const char *mac){
	
	if (ssid == NULL || pwd == NULL)
        return false;
	
	char cmd[256]; 	
	int len=snprintf(cmd,sizeof(cmd),"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,pwd);
	if(mac){
		snprintf(cmd+len,sizeof(cmd)-len,",\"%s\"\r\n",mac);
	
	}
	return esp_at_write_command(cmd,3000);

}



//wifi-3-1指令回显判断，类似于前面的receive,这里主要使用state的回显来判断前面连接后回显数据的成功还是失败
//AP指令也是只判断了OK
bool esp_at_get_wifi_info(esp_wifi_info_t *info){
	
	
	if( !esp_at_write_command("AT+CWSTATE?\r\n",1000)){//state下判断了OK
			return false;
	}
	
	/**state数据解析---也就是提取响应数据的信息*/
	if(!parse_cwstate_response(esp_at_get_response(),info)){//state下判断了state其他回显数据并提取
		return false;
	}
	
	if(info->connected == true){
	
			if( !esp_at_write_command("AT+CWJAP?\r\n",3000) ){//info详情下仅判断了OK
			return false;
	}
	
			/**AP数据解析---也就是提取响应数据的信息*/
			if( !parse_swjap_response(esp_at_get_response(),info) ){//AP下判断了AP其他回显数据并提取
					return false;
			}
	}
	
	
	return true;
}
//回显指令---connect的专门判断（类似ready），调用esp_at_get_wifi_info
bool wifi_is_connected(void){

	esp_wifi_info_t info;
	if(esp_at_get_wifi_info(&info)){
			return info.connected;
	}

	return false;
}






//wifi-4---这里额外在执行true的前提下返回接收的日志---wifi的响应数据解析单独放在了weather.c
const char *esp_at_http_get(const char *url){
//AT+HTTPCLIENT=2,0,"https://www.espressif.com/sites/all/themes/espressif/images/about-us/solution-platform.jpg",,,2
	
	char cmd[256]; 	
	snprintf(cmd,sizeof(cmd),"AT+HTTPCLIENT=2,0,\"%s\",,,2\r\n",url);
	bool ret=esp_at_write_command(cmd,5000);
	return ret? esp_at_get_response() : NULL;//command响应的数据会填入arr但是遇到OK就停
	
	
}



/***************************************************************/
//时间获取

//SNTP设置
bool esp_at_sntp_init(void){
	
	
	if( !esp_at_write_command("AT+CIPSNTPCFG=1,8\r\n",1000)){
			return false;
	}
	
	
	return true;
}

//获取时间
bool esp_at_sntp_get_time(esp_date_time_t *date){
	
	
	if( !esp_at_write_command("AT+CIPSNTPTIME?\r\n",1000)){//查询时间指令 会返回时间数据 需要判断并解析
			return false;
	}
	
	
	if( !parse_cipsntptime_response(esp_at_get_response(),date) ){//对响应的时间数据进行判断并提取出来到结构体
			return false;
	}
	
	return true;
}
/************************************************************/

//espat的复位操作
void esp_hard_reset(void) {
    // 1. 拉低 RST 引脚，按住 ESP32 的头让它重启
    GPIO_ResetBits(ESP_RST_PORT, ESP_RST_PIN);
    vTaskDelay(pdMS_TO_TICKS(100)); // 按住 100ms
    
    // 2. 松开 RST，让 ESP32 开始启动
    GPIO_SetBits(ESP_RST_PORT, ESP_RST_PIN);
    
    // 3. 【关键】等待 ESP32 启动完成（它会吐一堆乱码日志），我们死等 2秒
    // 这段时间绝对不要发 AT 指令，发了也是报错
    vTaskDelay(pdMS_TO_TICKS(2000)); 
    
    // 4. 【关键】清空 STM32 串口接收缓冲区
    // 把刚才 ESP32 吐出来的启动乱码全扔掉，防止干扰后续解析
    // 假设用的是 USART1
    while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {
        volatile uint8_t temp = USART_ReceiveData(USART1);
    }
}

/***********************************************************/

void USART1_IRQHandler(void){

	
		//判断是否接收成功
		if(arr_len < sizeof(arr)-1){
			
			if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == SET){
					USART_ClearFlag(USART1,USART_FLAG_RXNE);
					arr[arr_len++]=USART_ReceiveData(USART1);
					
					if(arr[arr_len-1]=='\n'){
						
							
							at_ack_t ack=match_internal_ack(line);
							if(ack != AT_ACK_NONE){
								

										rxack = ack;
								
										BaseType_t pxHigherPriorityTaskWoken;
										xSemaphoreGiveFromISR(at_ack_sempahore,&pxHigherPriorityTaskWoken);
										//切换任务	
										portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

							}
							line=arr+arr_len;
				
					}
			 		
			}	
		
		}

}













