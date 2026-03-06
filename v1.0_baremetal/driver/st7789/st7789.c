#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "cpu_tick.h"
#include "st7789.h"
#include "font.h"
#include "image.h"


//MISO暂时不引入，因为操作LCD主要是w
//LCD涉及的线CS、MOSI、MISO（暂时不写）、SCK、RESET、DC、BL
#define CS_PORT GPIOB
#define CS_PIN GPIO_Pin_12
#define MOSI_PORT GPIOB
#define MOSI_PIN GPIO_Pin_15
#define SCK_PORT GPIOB
#define SCK_PIN GPIO_Pin_13
#define RESET_PORT GPIOB
#define RESET_PIN GPIO_Pin_9
#define DC_PORT GPIOB
#define DC_PIN GPIO_Pin_10
#define BL_PORT GPIOB
#define BL_PIN GPIO_Pin_11

#define delay_us(x) cpu_delay_us(x)
#define delay_ms(x) cpu_delay_ms(x)


void st7789_init(void){

	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;
	
	GPIO_SetBits(GPIOB,GPIO_Pin_9 |GPIO_Pin_10 |GPIO_Pin_12 );
	GPIO_ResetBits(BL_PORT,BL_PIN);
	
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_13;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2); 
	
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_11;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	
	SPI_InitTypeDef  SPI_InitStruct; 
	SPI_StructInit(&SPI_InitStruct);
	SPI_InitStruct.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_4;//走的APB1总线=默认是42MHz
	SPI_InitStruct.SPI_CPHA=SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_DataSize=SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit=SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode=SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS=SPI_NSS_Soft;
	
	SPI_Init(SPI2,&SPI_InitStruct);
	
	SPI_Cmd(SPI2,ENABLE);


	st7789_init_display();
}







void st7789_reset(void){

	GPIO_ResetBits(RESET_PORT,RESET_PIN);
	delay_us(20);
	
	GPIO_SetBits(RESET_PORT,RESET_PIN);
	delay_ms(120);
	


}

void st7789_write_register(uint8_t reg,uint8_t arr[],uint16_t length){
	
	GPIO_ResetBits(CS_PORT,CS_PIN);
	GPIO_ResetBits(DC_PORT,DC_PIN);
	while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
	SPI_SendData(SPI2,reg);
	
	while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
	// 【关键修改】必须等待 SPI 忙碌结束，才能去切换 DC 引脚！
  // 如果不加这句，命令发到一半 DC 就变高了，屏幕就无法识别命令
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	
	GPIO_SetBits(DC_PORT,DC_PIN);
	uint8_t i=0; 
	for(i=0;i<length;i++){
		
		while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
		SPI_SendData(SPI2,arr[i]);
	
	}
	// 【关键修改】同样，拉高 CS 之前，必须确保最后一个字节发完了
	while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
  while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	
	GPIO_SetBits(CS_PORT,CS_PIN);

}



void st7789_set_backlight(bool on){

	GPIO_WriteBit(BL_PORT,BL_PIN,on ? Bit_SET:Bit_RESET);

}



void st7789_init_display(void){
	
	
	st7789_reset();

	//TFT_SEND_CMD(0x01);//Software Reset
	//delay_ms(120);

  st7789_write_register(0x11,NULL,0); 			//Sleep Out
	delay_ms(120);               //DELAY120ms
	 	  //-----------------------ST7789V Frame rate setting-----------------//
//************************************************
	st7789_write_register(0x3A,(uint8_t[]){0x55},1);       //65k mode	
	st7789_write_register(0xC5,(uint8_t[]){0x1A},1); 		//VCOM	
	st7789_write_register(0x36,(uint8_t[]){0x00},1);                 // 屏幕显示方向设置
	
	
	//-------------ST7789V Frame rate setting-----------//
	st7789_write_register(0xb2, (uint8_t[]){0x05,0x05,0x00,0x33,0x33}, 5);

	// Gate Control
	st7789_write_register(0xb7, (uint8_t[]){0x05}, 1);
	
	//--------------ST7789V Power setting---------------//
	// Porch Setting
	   // 注意：Gate Control 是 0xB7，不是 B2

//-------------- ST7789V Power setting ---------------//

	// VCOM
	st7789_write_register(0xBB, (uint8_t[]){0x3F}, 1);

	// Power Control
	st7789_write_register(0xC0, (uint8_t[]){0x2c}, 1);

	// VDV and VRH Command Enable
	st7789_write_register(0xC2, (uint8_t[]){0x01}, 1);

	// VRH Set
	st7789_write_register(0xC3, (uint8_t[]){0x0F}, 1);

	// VDV Set
	st7789_write_register(0xC4, (uint8_t[]){0x20}, 1);

	// Frame Rate Control in Normal Mode
	st7789_write_register(0xC6, (uint8_t[]){0x01}, 1);

	// Power Control 1
	st7789_write_register(0xD0, (uint8_t[]){0xA4, 0xA1}, 2);

	// Power Control 2
	st7789_write_register(0xE8, (uint8_t[]){0x03}, 1);

	// Equalize time control
	st7789_write_register(0xE9, (uint8_t[]){0x09,0x09,0x08}, 3);

	//--------------- ST7789V Gamma setting ---------------//

	// Positive Gamma
	st7789_write_register(0xE0, (uint8_t[]){
			0xD0,0x05,0x09,0x09,0x08,
			0x14,0x28,0x33,0x3F,
			0x07,0x13,0x14,0x28,0x30
	}, 14);

	// Negative Gamma
	st7789_write_register(0xE1, (uint8_t[]){
			0xD0,0x05,0x09,0x09,0x08,
			0x03,0x24,0x32,0x32,
			0x3B,0x14,0x13,0x28,0x2F
	}, 14);

	
	st7789_write_register(0x20, NULL, 0);		//反显
	delay_ms(120); 
	st7789_write_register(0x29, NULL, 0);         //开启显示

	
	
	st7789_fill_color(0,0,ST7789_WIDTH-1,ST7789_HEIGHT-1,0x0000);
	st7789_set_backlight(true);//背光开启
	
}


static bool in_screen_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if (x1 >= ST7789_WIDTH || y1 >= ST7789_HEIGHT)
        return false;
    if (x2 >= ST7789_WIDTH || y2 >= ST7789_HEIGHT)
        return false;
    if (x1 > x2 || y1 > y2)
        return false;

    return true;
}


static void st7789_set_range_and_prepare_gram(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    st7789_write_register(0x2A, (uint8_t[]){(x1 >> 8) & 0xff, x1 & 0xff, (x2 >> 8) & 0xff, x2 & 0xff}, 4);
    st7789_write_register(0x2B, (uint8_t[]){(y1 >> 8) & 0xff, y1 & 0xff, (y2 >> 8) & 0xff, y2 & 0xff}, 4);
		st7789_write_register(0x2C, NULL, 0);
}






//颜色填充--采用565的颜色
void st7789_fill_color(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color){
	
	if(!in_screen_range(x1,y1,x2,y2)){
		return;
	}

	st7789_set_range_and_prepare_gram(x1, y1, x2, y2);

	GPIO_ResetBits(CS_PORT,CS_PIN);
	GPIO_SetBits(DC_PORT,DC_PIN);
	
	uint32_t pixels=(x2-x1+1)*(y2-y1+1);
	uint8_t color_H=(color>>8)& 0xff;
	uint8_t color_L=color & 0xff;
	
	uint32_t i=0; 
	for(i=0;i<pixels;i++){
		
		while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
		SPI_SendData(SPI2,color_H);
		while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
		SPI_SendData(SPI2,color_L);
	
	}
	
	while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	
	GPIO_SetBits(CS_PORT,CS_PIN);

}





static void st7789_draw_font(uint16_t x1,uint16_t y1,uint16_t width, uint16_t height,const uint8_t *model,uint16_t color,uint16_t bg_color){

	
	st7789_set_range_and_prepare_gram(x1,y1,x1+width-1,y1+height-1);
	
	uint8_t color_data[2] = {(color >> 8) & 0xff, color & 0xff};
	uint8_t bg_color_data[2] = {(bg_color >> 8) & 0xff, bg_color & 0xff};
		//下面这块反正就是按着设定好的宽高去依次将数组的值填充到屏幕
	uint16_t bytes_per_row=(width+7)/8;
	
	GPIO_ResetBits(CS_PORT,CS_PIN);
	GPIO_SetBits(DC_PORT,DC_PIN);
	
	for (uint16_t row = 0; row < height; row++)
	{
		const uint8_t *row_data = model + row * bytes_per_row;
		for (uint16_t col = 0; col < width; col++)
		{
			uint8_t pixel = row_data[col / 8] & (1 << (7 - col % 8));
			if(pixel){//如果当前点阵的这个点是1就画颜色,否则画背景色
					
				while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);//高位先行
				SPI_SendData(SPI2,color_data[0]);
				while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
				SPI_SendData(SPI2,color_data[1]);
				
			}else{
			
					while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);//高位先行
					SPI_SendData(SPI2,bg_color_data[0]);
					while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
					SPI_SendData(SPI2,bg_color_data[1]);
			
			}
		}
	}
	
	while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	
	GPIO_SetBits(CS_PORT,CS_PIN);





}





//单字符填写。其中颜色填充--采用565的颜色
void st7789_write_ascii(uint16_t x1,uint16_t y1,const char c,uint16_t color,uint16_t bg_color,const font_t *font){
	
	if (font == NULL)
        return;
	
	uint16_t fheight=font->height,fwidth=font->height / 2;
	
	if(!in_screen_range(x1,y1,x1+fwidth-1,y1+fheight-1)){
		return;
	}

	if (c < 0x20 || c > 0x7E)
        return;
	
	//先拿到当前字符的起始地址  
	//当前地址=(当前字符ascii-空格ascii)即中间相隔多少字符，然后每个字符占着36row * 2Byte的点阵，即高度*2
	//原本代码是const uint8_t *model =font->ascii_model+(c - ' ') * fheight * bytes_per_row;
	//使用最新的囊括映射的方法
	const uint8_t *model =ascii_get_model(c,font);
	st7789_draw_font(x1,y1,fwidth-1,fheight-1,model,color,bg_color);
}



//单汉字填写。其中颜色填充--采用565的颜色
//特别注意的是汉字是全宽，而字符是半宽，
//以及因为未设置字符集既而不能使用偏移的方法为了随便输入几个汉字能显示出来这里使用cmp的方法
void st7789_write_chinese(uint16_t x1,uint16_t y1,const char *c,uint16_t color,uint16_t bg_color,const font_t *font){
	
	if (c == NULL || font == NULL)
        return;
	
	uint16_t fheight=font->height,fwidth=font->height;
	
	if(!in_screen_range(x1,y1,x1+fwidth-1,y1+fheight-1)){
		return;
	}

	//查表判断当前 传入的汉字是表里面的哪个，然后既而就得到了点阵数据
	//先拿到当前字符的在数组里面的起始地址
	const font_chinese_t *chinese_model =font->chinese_model;
	//这里就是因为“嵌入式”填充失败即只显示“式”这个字符想到的办法，”式“占两字节所以设置了两指针
	const char temp[3] = { *c, *(c+1), '\0' };//这里是我自己的方法解决了嵌入式只显示式的问题，视频里面采用了cpy
	for(;chinese_model->name!=NULL;chinese_model++){
		
		if(strcmp(chinese_model->name,temp)==0){
			break;
		}
	
	}
	if(chinese_model->name == NULL){
		return;
	}
	
	
	st7789_draw_font(x1,y1,fwidth-1,fheight-1,chinese_model->model,color,bg_color);
	

}


static bool is_gb2312(const char str){
	
	return ((unsigned char)str >=0xA1 && (unsigned char)str <=0xf7);


}


//UTF-8的判断
//static int utf8_char_length(const char *str)
//{
//    if ((*str & 0x80) == 0) return 1; // 1 byte
//    if ((*str & 0xE0) == 0xC0) return 2; // 2 bytes
//    if ((*str & 0xF0) == 0xE0) return 3; // 3 bytes
//    if ((*str & 0xF8) == 0xF0) return 4; // 4 bytes
//    return -1; // Invalid UTF-8
//}



void st7789_write_string(uint16_t x, uint16_t y,const char *str, uint16_t color, uint16_t bg_color, const font_t *font)
{
    while (*str)
    {
        // int len = utf8_char_length(*str);
        int len = is_gb2312(*str) ? 2 : 1;
        if (len <= 0)
        {
            str++;
            continue;
        }
        else if (len == 1)
        {
            st7789_write_ascii(x, y, *str, color, bg_color, font);
            str++;
            x += font->height / 2;
        }
        else
        {
            char ch[5]={0};
            strncpy(ch, str, len);//这里很关键，这里是视频里面的：解决了嵌入式只显示式的问题
            st7789_write_chinese(x, y, ch, color, bg_color, font);
            str += len;//注意GB2302下中文占2字节
            x += font->height;
        }
    }
}


//显示图片
void st7789_write_image(uint16_t x1,uint16_t y1,const image_t *image){
	
	if ( image == NULL)
        return;
	
	uint16_t height=image->height,width=image->width;
	
	if(!in_screen_range(x1,y1,x1+width-1,y1+height-1)){
		return;
	}
	st7789_set_range_and_prepare_gram(x1,y1,x1+width-1,y1+height-1);
	
	
	uint32_t data_cnt=image->height * image->width * 2;
 	
	GPIO_ResetBits(CS_PORT,CS_PIN);
	GPIO_SetBits(DC_PORT,DC_PIN);
	
	for (uint32_t i = 0; i < data_cnt; i+=2)
	{
		
				//注意这里因为产生的数据是小端存储的所以采用对于一个颜色先取后一个字节再取前一个
				while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);//高位先行.
				SPI_SendData(SPI2,image->data[i+1]);
				while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
				SPI_SendData(SPI2,image->data[i]);
				
			
	}
	
	while(SPI_GetFlagStatus(SPI2,SPI_FLAG_TXE)==RESET);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
	
	GPIO_SetBits(CS_PORT,CS_PIN);
	
}


/********************************/
//获取某个字符的方法---映射法；在字符串里面一个个比，如果这个字符匹配了就直接找到这个字符
//找到这个字符对应的在点阵数据的位置，比如映射字符串为”123“，如果找到”3“那就直接取第三个位置的点阵数据显示出来就行
static const uint8_t *ascii_get_model(const char ch,const font_t *font){
	
	uint16_t bytes_per_row=((font->height/2)+7)/8;//一行的字节数:(width+7)/8;因为是字符所以是半宽即height/2
	uint16_t byte_per_char= font->height * bytes_per_row ;//一个字符的点阵大小
	
	if(font->ascii_map){//如果是准备走映射的方法，那么就进入if 否则就正常的走ascii的字符集的偏移方法
		
		const char *map=font->ascii_map;
		do{
		
			if(*map == ch){
				//如果找到匹配的，就直接返回当前字符的点阵数据
				//当前字符的数据地址在：点阵数据的起始地址+映射字符串中当前匹配的字符 离最初字符的差距 再乘上一个字符占据的点阵数据大小
				//而一个字符占据的点阵数据大小=height * 一行字节数
				return font->ascii_model + (map - font->ascii_map) * byte_per_char;
			
			}
		
		}while( *(++map)!='\0');  //这里其实就是map地址后移一字节，然后取值判断是否为\0，注意如果是中文就需要+2Byte
	}else{
			

			return font->ascii_model + ( ch - ' ') * byte_per_char;
	
	
	}
	
	return NULL;

}
