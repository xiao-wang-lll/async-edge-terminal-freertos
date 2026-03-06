#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "font.h"
#include "image.h"
#include "workqueue.h"


typedef struct
{
    work_t work;
    void *param;
} work_message_t;



/**
*使用队列的方法去依次运行多个模块
*1.回调 2.init 3.run  4.run中执行所需要的结构体对象
**/
static QueueHandle_t work_msg_queue;


//专门用来处理ui的function
static void work_func(void *param){

	work_message_t msg;
	
	while(1){
		
		xQueueReceive(work_msg_queue,&msg,portMAX_DELAY);
		msg.work(msg.param);
	
	
	}

}




//工作队列
void workqueue_init(void){

	work_msg_queue=xQueueCreate(16,sizeof(work_message_t));
	configASSERT(work_msg_queue);
	
	xTaskCreate(work_func,"work_queue",1024,NULL,5,NULL);


}

//既然是要run 那些回调的其他函数，就要接收回调函数名称和参数，才能在这里执行，所以需要这样的结构体对象
void workqueue_run(work_t work, void *param){

	configASSERT(work_msg_queue);//防止往空队列塞数据
	//将对应的匹配回调的函数名称和参数拿到
	work_message_t temp= {work,param};
	
	xQueueSend(work_msg_queue,&temp,portMAX_DELAY);//放到队列里面


}
