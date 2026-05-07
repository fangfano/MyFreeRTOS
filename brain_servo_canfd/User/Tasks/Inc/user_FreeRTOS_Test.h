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
extern osMessageQueueId_t Test_MessageQueue;

/* 测试初始化函数 */
void FreeRTOS_Test_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USER_FREERTOS_TEST_H__ */
