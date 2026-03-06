#include <elog.h>
#include "rtc.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include "console.h"

static bool elog_inited; 

static SemaphoreHandle_t elog_mutex;
static SemaphoreHandle_t elog_semphr; // 新增：用于通知后台任务“有新日志了”
static void async_output(void * arg);


ElogErrCode elog_port_init(void) {
    
    BaseType_t result;

    if (elog_inited) {
        return ELOG_NO_ERR;
    }

    elog_mutex = xSemaphoreCreateMutex();
    if (elog_mutex == NULL) {
        printf("Error: EasyLogger mutex create failed\r\n");
        goto err1;
    }

    // 【新增】创建一个二值信号量
    elog_semphr = xSemaphoreCreateBinary();
    if (elog_semphr == NULL) {
        printf("Error: EasyLogger semaphore create failed\r\n");
        goto err2;
    }

    // 【新增】创建一个专门负责发日志的 FreeRTOS 任务
    result = xTaskCreate(async_output, "elog", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
    if (result != pdPASS) {
        printf("Error: EasyLogger async output task create failed\r\n");
        goto err3;
    }

    elog_inited = true;
    return ELOG_NO_ERR;

err3:
    vSemaphoreDelete(elog_semphr);
    elog_semphr = NULL;
err2:
    vSemaphoreDelete(elog_mutex);
    elog_mutex = NULL;
err1:
    return ELOG_HAS_ERR;
}


void elog_port_deinit(void) {

    if (elog_semphr != NULL) {
        vSemaphoreDelete(elog_semphr);
        elog_semphr = NULL;
    }
    if (elog_mutex != NULL) {
        vSemaphoreDelete(elog_mutex);
        elog_mutex = NULL;
    }
}


void elog_port_output(const char *log, size_t size) {
    
    console_write(log, size);
    
}


void elog_port_output_lock(void) {
    
    if (elog_mutex != NULL)
    {
        xSemaphoreTake(elog_mutex, portMAX_DELAY);
    }
}
    


void elog_port_output_unlock(void) {
    
    if (elog_mutex != NULL)
    {
        xSemaphoreGive(elog_mutex);
    }
    
}


const char *elog_port_get_time(void) {
    
    static char time_str[32];
    rtc_date_time_t time;
    rtc_get_time(&time);
    if (time.year >= 2000 && time.year <= 2099)
    {
        snprintf(time_str, sizeof(time_str), "%02d-%02d-%02d %02d:%02d:%02d",
                 time.year % 100, time.month, time.day,
                 time.hour, time.minute, time.second);
        return time_str;
    }
    else
    {
        snprintf(time_str, sizeof(time_str), "%lu", xTaskGetTickCount());
        return time_str;
    }
    
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    
    return "";
    
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    
    if (xTaskGetCurrentTaskHandle() != NULL)
    {
        return pcTaskGetName(NULL);//NULL表示获取当前任务
    }
    else
    {
        return "none";
    }
    
}

// EasyLogger 内部通知接口：告诉你内存里有新日志了
void elog_async_output_notice(void)
{
    if (elog_semphr != NULL) {
        xSemaphoreGive(elog_semphr); // 释放信号量，唤醒后台任务
    }
}

// 专属后台任务：默默搬砖把内存里的日志发到串口
static void async_output(void *arg)
{
    size_t log_size = 0;
    char log_buff[ELOG_LINE_BUF_SIZE];

    while (true)
    {
        // 死等信号量，没有新日志就挂起，绝对不占用 CPU
        xSemaphoreTake(elog_semphr, portMAX_DELAY);

        do
        {
#ifdef ELOG_ASYNC_LINE_OUTPUT
            log_size = elog_async_get_line_log(log_buff, ELOG_LINE_BUF_SIZE);
#else
            // 把积压在内部 RingBuffer 里的日志一次性取出来
            log_size = elog_async_get_log(log_buff, ELOG_LINE_BUF_SIZE);
#endif

            if (log_size > 0)
                elog_port_output(log_buff, log_size); // 底层调用你的 console_write

        } while (log_size > 0); // 只要还有数据，就一直掏空
    }
}


