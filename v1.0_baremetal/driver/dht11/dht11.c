#include "stm32f4xx.h"                  // Device header
#include  "dht11.h"
#include  "cpu_tick.h"
#include "st7789.h"
#include "console.h"


//数据
float rec_data[4];


//对于stm32来说，是输出
void DH11_GPIO_Init_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT; //推挽输出
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOD, &GPIO_InitStructure);

}

//对于stm32来说，是输入
void DH11_GPIO_Init_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN; //浮空输入
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOD, &GPIO_InitStructure);

}



//主机发送开始信号
void DHT11_Start(void)
{
	DH11_GPIO_Init_OUT(); //输出模式
	
	dht11_high; //先拉高
	cpu_delay_us(30);
	
	dht11_low; //拉低电平至少18us
	cpu_delay_ms(20);
	
	dht11_high; //拉高电平20~40us
	cpu_delay_us(30);
	
	DH11_GPIO_Init_IN(); //输入模式,即前面主机唤醒从机，此时开始接收从机DHT11数据
}


//获取一个字节
char DHT11_Rec_Byte(void)
{
	unsigned char i = 0;
	unsigned char data = 0;
	
	for(i=0;i<8;i++) //1个数据就是1个字节byte，1个字节byte有8位bit
	{
		while( Read_Data == 0); //从1bit开始，低电平变高电平，等待低电平结束
		cpu_delay_us(30); //延迟30us是为了区别数据0和数据1，0只有26~28us
		
		data <<= 1; //左移
		
		if( Read_Data == 1 ) //如果过了30us还是高电平的话就是数据1
		{
			data |= 1; //数据+1
		}
		
		while( Read_Data == 1 ); //高电平变低电平，等待高电平结束
	}
	
	return data;
}

//获取数据

void DHT11_REC_Data(float *temperature,float *humidity)  //这里为了前后数据对比，自己设计了传入参数
{
	unsigned int R_H,R_L,T_H,T_L;
	unsigned char RH,RL,TH,TL,CHECK;
	
	DHT11_Start(); //主机发送信号
	dht11_high; //强制再次拉高电平作为保险，因为后续DHT11要实现拉低响应
	
	if( Read_Data == 0 ) //判断DHT11是否响应
	{
		while( Read_Data == 0); //低电平变高电平，等待低电平结束
		while( Read_Data == 1); //高电平变低电平，等待高电平结束
		
		R_H = DHT11_Rec_Byte();
		R_L = DHT11_Rec_Byte();
		T_H = DHT11_Rec_Byte();
		T_L = DHT11_Rec_Byte();
		CHECK = DHT11_Rec_Byte(); //接收完5个数据
		
		dht11_low; //当最后一bit数据传送完毕后，DHT11拉低总线 50us
		cpu_delay_us(55); //这里延时55us
		dht11_high; //随后总线由上拉电阻拉高进入空闲状态。
		
		if(R_H + R_L + T_H + T_L == CHECK) //和检验位对比，判断校验接收到的数据是否正确
		{
			RH = R_H;
			RL = R_L;
			TH = T_H;
			TL = T_L;
		}
	}
	rec_data[0] = RH;
	rec_data[1] = RL;
	rec_data[2] = TH;
	rec_data[3] = TL;
	
	*temperature =rec_data[2];
	*humidity = rec_data[0];
	
	
}




//extern float rec_data[4];
//注意这个示例 的内容可以仅使用串口的方式在main中实现
//void dht11_simple(void)
//{
	
//		cpu_delay_ms(1000);
//		DHT11_REC_Data(); //接收温度和湿度的数据
//		

//		printf("temperature:%.0f\r\n",rec_data[2]); 
//		printf("humidity:%.0f\r\n",rec_data[0]);
//    printf("\r\n\n");	
//}





