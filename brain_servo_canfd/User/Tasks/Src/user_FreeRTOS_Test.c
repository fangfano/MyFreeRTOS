#include "user_FreeRTOS_Test.h"
#include <stdio.h> // 使用 printf 进行串口打印
#include "main.h"

/* 消息队列句柄 */
osMessageQueueId_t Test_MessageQueue;

/* ------------------------------------------------------------------
 * 任务控制块与属性定义
 * ------------------------------------------------------------------ */
osThreadId_t TestSendTaskHandle;
const osThreadAttr_t TestSendTask_attributes = {
  .name = "TestSendTask",
  .stack_size = 128 * 16, // printf非常吃栈空间，所以这里分配空间一定要大一些
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t TestRecvTaskHandle;
const osThreadAttr_t TestRecvTask_attributes = {
  .name = "TestRecvTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};

/* ------------------------------------------------------------------
 * 任务函数实体
 * ------------------------------------------------------------------ */

/**
 * @brief 发送任务：周期性向队列中投递消息
 */
void Test_QueueSendTask(void *argument)
{

	// 任务刚开始运行，说明调度器已经启动，任务创建成功  // 这里可以有打印，因为调度器已经启动
	printf("\r\n[TEST] Message Queue and Tasks created successfully.\r\n");
    TestMsg_t send_msg = {0, 100}; // 初始化消息内容

    while (1) {
        send_msg.msg_id++;
        send_msg.data_value += 5; // 模拟数据变化

        // 尝试将消息放入队列，超时时间设为 0 (不阻塞等待)
        osStatus_t status = osMessageQueuePut(Test_MessageQueue, &send_msg, 0, 0);

        if (status == osOK) {
            printf("[Test Send Task] Sent Msg ID: %lu, Value: %lu\r\n", send_msg.msg_id, send_msg.data_value);
        } else {
            printf("[Test Send Task] Queue Full! Failed to send.\r\n");
        }

        osDelay(2000); // 延时 2 秒，观察打印效果
    }
}

/**
 * @brief 接收任务：阻塞等待队列中的消息
 */
void Test_QueueRecvTask(void *argument)
{
    TestMsg_t recv_msg;

    while (1) {
        // 等待消息队列，超时时间设为 osWaitForever (死等，直到有消息进入)
        osStatus_t status = osMessageQueueGet(Test_MessageQueue, &recv_msg, NULL, osWaitForever);

        if (status == osOK) {
            printf("[Test Recv Task] Received Msg ID: %lu, Value: %lu\r\n", recv_msg.msg_id, recv_msg.data_value);
        }
    }
}

/* ------------------------------------------------------------------
 * 初始化入口
 * ------------------------------------------------------------------ */
void FreeRTOS_Test_Init(void)
{
    // 创建一个包含 5 个元素的消息队列，每个元素的大小是 TestMsg_t
    Test_MessageQueue = osMessageQueueNew(5, sizeof(TestMsg_t), NULL);

    if (Test_MessageQueue != NULL) {
        // 队列创建成功，创建发送和接收任务 //
        TestSendTaskHandle = osThreadNew(Test_QueueSendTask, NULL, &TestSendTask_attributes);
        TestRecvTaskHandle = osThreadNew(Test_QueueRecvTask, NULL, &TestRecvTask_attributes);
        // 这里千万不能有打印，会导致程序，因为这里在 osKernelStart() 被调用之前，所有的 RTOS 阻塞 API 都是非法的
//        printf("\r\n[TEST] Message Queue and Tasks created successfully.\r\n");
    } else {
//        printf("\r\n[TEST] Failed to create Message Queue!\r\n");
    	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    }
}
