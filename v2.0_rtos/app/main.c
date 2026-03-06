#include "page.h"
#include "app.h"
#include "board.h"
#include "FREERTOS.h"
#include "task.h"
#include "ui.h"
#include "workqueue.h"
#include "elog.h"

static void main_init(void *param){
		
	
	board_init();
	
	
	 elog_init();
	elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
	elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL);
	elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_T_INFO);
	elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
	elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG);
	elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG);
	elog_async_enabled(true);
	elog_start();
	
	ui_init();//为了防止time更新和其他几个同步模块更新造成的刷屏冲突
	welcome_page_display();
	
	wifi_init();
	wifi_page_display();
	wifi_wait_connect();
	
	main_page_display();
	app_init();
	
	vTaskDelete(NULL);



}

int main(){

	
	board__lowlevel_init();
	workqueue_init();//工作队列的初始化，即app的几大功能模块的队列初始化
	
	xTaskCreate(main_init,"init",1024,NULL,5,NULL);
	
	vTaskStartScheduler();

	while (1)
    {
        ; // code should not run here
    }

}







