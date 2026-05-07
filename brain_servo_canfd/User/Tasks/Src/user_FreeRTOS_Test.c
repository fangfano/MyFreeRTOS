#include "user_FreeRTOS_Test.h"
#include <stdio.h> // 使用 printf 进行串口打印
#include "main.h"
#include <stdarg.h>

/* 消息队列句柄 */
osMessageQueueId_t Test_MessageQueue; // 消息队列

osSemaphoreId_t    Test_BinarySema; // 二值信号量
osSemaphoreId_t    Test_CountingSema; // 计数量

osMutexId_t        Test_UartMutex; // 互斥量

osEventFlagsId_t Test_EventGroup; // 事件组

osTimerId_t Test_PeriodicTimer; // 周期的软件定时器
osTimerId_t Test_OneShotTimer; // 单次的软件定时器

/* ------------------------------------------------------------------
 * 任务控制块与属性定义
 * ------------------------------------------------------------------ */
// 消息队列 发送
osThreadId_t TestSendTaskHandle;
const osThreadAttr_t TestSendTask_attributes = {
  .name = "TestSendTask",
  .stack_size = 128 * 16, // printf非常吃栈空间，所以这里分配空间一定要大一些
  .priority = (osPriority_t) osPriorityNormal,
};
// 消息队列 接收
osThreadId_t TestRecvTaskHandle;
const osThreadAttr_t TestRecvTask_attributes = {
  .name = "TestRecvTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};

// 信号量发送测试
osThreadId_t SemaGiveTaskHandle;
const osThreadAttr_t SemaGiveTask_attributes = {
  .name = "SemaGiveTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};
// 信号量接收测试
osThreadId_t SemaTakeTaskHandle;
const osThreadAttr_t SemaTakeTask_attributes = {
  .name = "SemaTakeTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};


// 互斥量专属测试任务
osThreadId_t MutexTaskAHandle;
const osThreadAttr_t MutexTaskA_attributes = {
  .name = "MutexTaskA",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t MutexTaskBHandle;
const osThreadAttr_t MutexTaskB_attributes = {
  .name = "MutexTaskB",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};


// 事件组专属测试任务
osThreadId_t EventSetTaskHandle;
const osThreadAttr_t EventSetTask_attributes = {
  .name = "EventSetTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t EventWaitTaskHandle;
const osThreadAttr_t EventWaitTask_attributes = {
  .name = "EventWaitTask",
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

/* ==================================================================
 * 测试任务 3 & 4：信号量测试 (二值 & 计数)
 * ================================================================== */
/**
 * @brief 释放信号量任务：每隔 3 秒开一次枪，并放入 3 辆车
 */
//--- [Sema Give] Action Triggered! ---  // 二值通知
//[Sema Take] Binary Semaphore Acquired! (Task Synced)
//[Sema Take] Counting Semaphore Acquired! (1/3) // 上面的打印结束后1s打印这个
//[Sema Take] Counting Semaphore Acquired! (2/3) // 上面的打印结束后1s打印这个
//[Sema Take] Counting Semaphore Acquired! (3/3) // 上面的打印结束后1s打印这个
void Test_SemaGiveTask(void *argument)
{
    while(1) {
        osDelay(3000); // 每 3 秒执行一次操作

        printf("\r\n--- [Sema Give] Action Triggered! ---\r\n");

        // 1. 释放一个二值信号量 (发令枪响)
        osSemaphoreRelease(Test_BinarySema);

        // 2. 连续释放三个计数信号量 (放出 3 个停车位)
        osDelay(1000); // 每 1 秒执行一次操作
        osSemaphoreRelease(Test_CountingSema);
        osDelay(1000); // 每 1 秒执行一次操作
        osSemaphoreRelease(Test_CountingSema);
        osDelay(1000); // 每 1 执行一次操作
        osSemaphoreRelease(Test_CountingSema);
    }
}

/**
 * @brief 获取信号量任务：阻塞等待信号量
 */
void Test_SemaTakeTask(void *argument)
{
    while(1) {
        // 1. 等待二值信号量 (等待发令枪)
        osSemaphoreAcquire(Test_BinarySema, osWaitForever);
        printf("[Sema Take] Binary Semaphore Acquired! (Task Synced)\r\n");

        // 2. 连续获取三次计数信号量 (停入 3 辆车)
        for(int i = 0; i < 3; i++) {
            osSemaphoreAcquire(Test_CountingSema, osWaitForever);
            printf("[Sema Take] Counting Semaphore Acquired! (%d/3)\r\n", i + 1);
        }
    }
}


/* ==================================================================
 * 测试任务 5 & 6：互斥量测试 (直观感受抢占与保护)
 * ================================================================== */
// 正常抢不了，A会全部输出结束后，B才能输出
//[Task A] Start: AAAAAAAAAAAAAAAAAAAA :End
//[Task B] Start: BBBBBBBBBBBBBBBBBBBB :End
void Test_MutexTaskA(void *argument)
{
    while(1) {
        // 【获取钥匙】死等互斥锁
        osMutexAcquire(Test_UartMutex, osWaitForever);

        printf("[Task A] Start: ");
        for(int i = 0; i < 20; i++) {
            printf("A");
            // 故意制造 50ms 延时。交出 CPU 控制权，引诱 Task B 来抢串口
            osDelay(50);
        }
        printf(" :End\r\n");

        // 【归还钥匙】释放互斥锁
        osMutexRelease(Test_UartMutex);

        osDelay(1000); // 休息 1 秒再开始下一轮
    }
}

void Test_MutexTaskB(void *argument)
{
    while(1) {
        // 【获取钥匙】死等互斥锁
        osMutexAcquire(Test_UartMutex, osWaitForever);

        printf("[Task B] Start: ");
        for(int i = 0; i < 20; i++) {
            printf("B");
            // 故意制造 50ms 延时。交出 CPU 控制权，引诱 Task A 来抢串口
            osDelay(50);
        }
        printf(" :End\r\n");

        // 【归还钥匙】释放互斥锁
        osMutexRelease(Test_UartMutex);

        osDelay(1000); // 休息 1 秒再开始下一轮
    }
}


// 事件组测试
//事件发送任务 (Set Task)： 模拟不同的传感器。第2秒设置 BIT_0，第4秒设置 BIT_1。
//事件接收任务 (Wait Task)： 阻塞等待。必须 同时等到 BIT_0 和 BIT_1 都被设置后，才开始执行，并打印提示。
/* ==================================================================
 * 测试任务 7 & 8：事件标志组测试 (等待多个条件满足)
 * ================================================================== */
/**
 * @brief 触发事件任务：模拟多源事件发生
 */
void Test_EventSetTask(void *argument)
{
    while(1) {
        osDelay(2000); // 等待 2 秒
        printf("\r\n[Event Set] Setting EVENT_BIT_0...\r\n");
        osEventFlagsSet(Test_EventGroup, EVENT_BIT_0); // 设置事件 0

        osDelay(2000); // 再等 2 秒
        printf("[Event Set] Setting EVENT_BIT_1...\r\n");
        osEventFlagsSet(Test_EventGroup, EVENT_BIT_1); // 设置事件 1

        // 两个事件都设置后，等待接收任务处理完毕，再重新开始
        osDelay(2000);
    }
}

/**
 * @brief 等待事件任务：必须同时满足多个条件才能继续
 */
void Test_EventWaitTask(void *argument)
{
    uint32_t wait_flags = EVENT_BIT_0 | EVENT_BIT_1; // 我们要等待这两个位
    uint32_t flags;

    while(1) {
        printf("[Event Wait] Waiting for BOTH Bit 0 and Bit 1...\r\n");

        /* * 阻塞等待事件组
         * osFlagsWaitAll: 逻辑与 (AND)，必须所有指定的位都被设置
         * osWaitForever: 死等
         * 默认行为：等待到之后，会自动清除这些标志位，方便下一次等待
         */
        flags = osEventFlagsWait(Test_EventGroup, wait_flags, osFlagsWaitAll, osWaitForever);

        if ((flags & wait_flags) == wait_flags) {
            printf(">>>> [Event Wait] BINGO! Both events occurred. Processing data...\r\n\r\n");
        }
    }
}


/* ==================================================================
 * 测试任务 9：软件定时器测试 (周期 vs 单次)
 * ================================================================== */
//启动瞬间打印 [TEST] Software Timers Started!
//随后每隔 1 秒，周期定时器打印一次：[Periodic Timer] Tick... 1, Tick... 2 ...
//当时间到达第 5 秒时，单次定时器突然杀出，打印一条华丽的：>>>> [One-Shot Timer] BOOM! 5 Seconds elapsed! <<<<。
//之后，单次定时器彻底沉寂，而周期定时器继续它每秒一次的报数。
/**
 * @brief 周期定时器回调函数 (每 1000ms 触发一次)
 */
void Test_PeriodicTimer_Callback(void *argument)
{
    static uint32_t tick_count = 0;
    tick_count++;

    // 注意：在定时器回调中直接用 printf 即可，详见下方的“避坑指南”
    printf("[Periodic Timer] Tick... %lu\r\n", tick_count);
}

/**
 * @brief 单次定时器回调函数 (启动后 5000ms 仅触发一次)
 */
void Test_OneShotTimer_Callback(void *argument)
{
    printf("\r\n>>>> [One-Shot Timer] BOOM! 5 Seconds elapsed! <<<<\r\n\r\n");
}

/* ------------------------------------------------------------------
 * 初始化入口
 * ------------------------------------------------------------------ */
void FreeRTOS_Test_Init(void)
{
	// 如果需要测试，就把这个0，改成1
	if(0){
		// 消息队列测试
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

	// 互斥信号量测试
	if(0){
		// 如果前面没创建过互斥量，这里需要创建一下
		if (Test_UartMutex == NULL) {
			Test_UartMutex = osMutexNew(NULL);
		}

		if (Test_UartMutex != NULL) {
			MutexTaskAHandle = osThreadNew(Test_MutexTaskA, NULL, &MutexTaskA_attributes);
			MutexTaskBHandle = osThreadNew(Test_MutexTaskB, NULL, &MutexTaskB_attributes);
		}
	}

	// 二值信号量、计数信号量测试
	if(0){
		/* 二值信号量、计数信号量测试 */
		/* 1. 创建二值信号量 (最大值1，初始值0) */
		Test_BinarySema = osSemaphoreNew(1, 0, NULL);
		/* 2. 创建计数信号量 (最大值3，初始值0) */
		Test_CountingSema = osSemaphoreNew(3, 0, NULL);
		/* 3. 创建 测试任务 */
		if (Test_UartMutex && Test_BinarySema && Test_CountingSema) {
			SemaGiveTaskHandle = osThreadNew(Test_SemaGiveTask, NULL, &SemaGiveTask_attributes);
			SemaTakeTaskHandle = osThreadNew(Test_SemaTakeTask, NULL, &SemaTakeTask_attributes);
		}
	}


	// 事件组测试
	if(0){
		/* 1. 创建事件标志组 */
		Test_EventGroup = osEventFlagsNew(NULL);

		/* 2. 创建 测试任务 */
		if (Test_EventGroup != NULL) {
			EventSetTaskHandle = osThreadNew(Test_EventSetTask, NULL, &EventSetTask_attributes);
			EventWaitTaskHandle = osThreadNew(Test_EventWaitTask, NULL, &EventWaitTask_attributes);
		}
	}


	// 软件定时器测试
	if(0){
		/* 1. 创建周期定时器 (osTimerPeriodic) */
		Test_PeriodicTimer = osTimerNew(Test_PeriodicTimer_Callback, osTimerPeriodic, NULL, NULL);

		/* 2. 创建单次定时器 (osTimerOnce) */
		Test_OneShotTimer = osTimerNew(Test_OneShotTimer_Callback, osTimerOnce, NULL, NULL);

		/* 3. 启动定时器 */
		if (Test_PeriodicTimer != NULL && Test_OneShotTimer != NULL) {

			// 启动周期定时器，周期为 1000 个 tick (因为 configTICK_RATE_HZ=1000，所以是 1000ms)
			osTimerStart(Test_PeriodicTimer, 1000);

			// 启动单次定时器，时长为 5000 个 tick (5000ms)
			osTimerStart(Test_OneShotTimer, 5000);
		}
	}


}
