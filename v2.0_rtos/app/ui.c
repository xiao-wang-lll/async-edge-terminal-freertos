#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "st7789.h"
#include "ui.h"
#include "font.h"
#include "image.h"


#define LOG_TAG "ui"
#define LOG_LVL ELOG_LVL_INFO
#include "elog.h"

/**
*使用队列的方法去解决多线程造成的同时调用刷屏方法
**/
static QueueHandle_t ui_queue;

//创建三个刷屏action
typedef enum
{
    UI_ACTION_FILL_COLOR,
    UI_ACTION_WRITE_STRING,
    UI_ACTION_DRAW_IMAGE,
} ui_action_t;


//当取到队列的某个刷屏action，就要调用驱动层st7789的对应刷屏操作,但是传参是一个问题
//所以需要一个结构体 既有action去匹配队列读出的行为  又有各种action的参数去调用st7789的刷屏函数
typedef struct{

	ui_action_t action;
	
	union{
		struct{uint16_t x1;uint16_t y1;uint16_t x2;uint16_t y2;uint16_t color;}fill_color;
		
		struct{uint16_t x; uint16_t y;const char *str; uint16_t color;uint16_t bg_color;const font_t *font;}write_string;
											
		struct{uint16_t x1;uint16_t y1;const image_t *image;}draw_image;
	};

}ui_message_t;


//任务函数实现不断取队列的值，专门用来处理ui的function
static void ui_func(void *param){

	ui_message_t msg;
	st7789_init();
	
	while(1){
		
		xQueueReceive( ui_queue,&msg,portMAX_DELAY);
    
		switch(msg.action){
		
			case UI_ACTION_FILL_COLOR : 
							st7789_fill_color(msg.fill_color.x1, msg.fill_color.y1,
                              msg.fill_color.x2, msg.fill_color.y2,
                              msg.fill_color.color); 
							break; 
			case UI_ACTION_WRITE_STRING : 
							st7789_write_string(msg.write_string.x, msg.write_string.y,
                                msg.write_string.str,
                                msg.write_string.color, msg.write_string.bg_color,
                                msg.write_string.font);
							vPortFree((void*)msg.write_string.str);
							break; 
			case UI_ACTION_DRAW_IMAGE : 
							st7789_write_image(msg.draw_image.x1, msg.draw_image.y1,
                              msg.draw_image.image);
							break; 

			default:	
						log_w("Unknown UI action: %d", msg.action);
            break;
		}
	}
}


void ui_init(void){
	ui_queue = xQueueCreate(16,sizeof(ui_message_t));
	configASSERT(ui_queue);
	
	xTaskCreate(ui_func,"ui_func",1024,NULL,8,NULL);//创建一个不断循环读取队列内容的任务

}


void ui_fill_color(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color){


		ui_message_t msg;
	
		msg.action = UI_ACTION_FILL_COLOR;
		msg.fill_color.x1 = x1;
		msg.fill_color.y1 = y1;
		msg.fill_color.x2 = x2;
		msg.fill_color.y2 = y2;
		msg.fill_color.color = color;

		xQueueSend(ui_queue,&msg,portMAX_DELAY);
}


void ui_write_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const font_t *font){
		
		char *pstr =pvPortMalloc(strlen(str)+1);
		if (pstr == NULL)
    {
        log_e("ui write string malloc failed: %s", str);
        return;
    }
		strcpy(pstr, str);

		ui_message_t msg;
    msg.action = UI_ACTION_WRITE_STRING;
    msg.write_string.x = x;
    msg.write_string.y = y;
    msg.write_string.str = pstr;
    msg.write_string.color = color;
    msg.write_string.bg_color = bg_color;
    msg.write_string.font = font;
    
    xQueueSend(ui_queue, &msg, portMAX_DELAY);
}


void ui_write_image(uint16_t x, uint16_t y, const image_t *image){


		ui_message_t msg;
    msg.action = UI_ACTION_DRAW_IMAGE;
    msg.draw_image.x1 = x;
    msg.draw_image.y1 = y;
    msg.draw_image.image = image;
    
    xQueueSend(ui_queue, &msg, portMAX_DELAY);
}







