#ifndef __USER_FREERTOS_TEST_H__
#define __USER_FREERTOS_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "cmsis_os2.h"

/* 定义测试用的消息结构体 */
typedef struct {
    uint32_t msg_id;      // 消息的流水号
    uint32_t data_value;  // 模拟的传感器数据或其他变量
} TestMsg_t;
/* 外部声明以便其他文件（如果需要）也能看到 */
extern osMessageQueueId_t Test_MessageQueue;  // 消息队列测试

extern osSemaphoreId_t    Test_BinarySema;       // 二值信号量
extern osSemaphoreId_t    Test_CountingSema;     // 计数信号量

extern osMutexId_t        Test_UartMutex;        // 串口互斥锁

extern osEventFlagsId_t   Test_EventGroup;       // 新增：事件标志组

extern osTimerId_t        Test_PeriodicTimer;    // 周期定时器
extern osTimerId_t        Test_OneShotTimer;     // 单次定时器

/* 新增：CPU 统计任务 */
extern osThreadId_t       Test_CpuStatsTaskHandle;

/* --- 事件标志位定义 (最多 31 位可用) --- */
#define EVENT_BIT_0  (1 << 0)  // 0x01
#define EVENT_BIT_1  (1 << 1)  // 0x02
#define EVENT_BIT_2  (1 << 2)  // 0x04

/* 测试初始化函数 */
void FreeRTOS_Test_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USER_FREERTOS_TEST_H__ */
