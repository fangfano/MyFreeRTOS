/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "uart_console.h"
//sys
#include "stdio.h"
#include "main.h"
#include "iwdg.h"

// tasks 定义了自定义的任务线程入口
// 对应的.c文件中有对应任务线程的具体内容实现
#include "user_HardwareInitTask.h"
#include "user_RunModeTasks.h"
#include "user_KeyTask.h"
#include "user_SensUpdateTask.h"
#include "user_DataSaveTask.h"
#include "user_ConsoleRecvTask.h"
// test demo
#include "user_FreeRTOS_Test.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
uint32_t life_count = 0;

/* Timers --------------------------------------------------------------------*/
osTimerId_t IdleTimerHandle; // 定义了一个定时器，用于计算系统进入休眠/暗屏的时间


/* Tasks ---------------------------------------------------------------------*/
// Hardwares initialization
// 定义了与线程相关的线程句柄以及线程控制块
osThreadId_t HardwareInitTaskHandle;
const osThreadAttr_t HardwareInitTask_attributes = {
  .name = "HardwareInitTask",
  .stack_size = 128 * 32,
  .priority = (osPriority_t) osPriorityHigh3,
};

//Idle Enter Task
osThreadId_t IdleEnterTaskHandle;
const osThreadAttr_t IdleEnterTask_attributes = {
  .name = "IdleEnterTask",
  .stack_size = 128 * 5,
  .priority = (osPriority_t) osPriorityHigh,
};

//Stop Enter Task
osThreadId_t StopEnterTaskHandle;
const osThreadAttr_t StopEnterTask_attributes = {
  .name = "StopEnterTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityHigh1,
};

//Key task
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityNormal,
};


//SensorDataRenew task
osThreadId_t SensorDataTaskHandle;
const osThreadAttr_t SensorDataTask_attributes = {
  .name = "SensorDataTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};


//messagesendtask
osThreadId_t MessageSendTaskHandle;
const osThreadAttr_t MessageSendTask_attributes = {
  .name = "MessageSendTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow1,
};


//DataSaveTask
osThreadId_t DataSaveTaskHandle;
const osThreadAttr_t DataSaveTask_attributes = {
  .name = "DataSaveTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityLow2,
};

//ConsoleRecvTask
osThreadId_t ConsoleRecvTaskHandle;
const osThreadAttr_t ConsoleRecvTask_attributes = {
  .name = "ConsoleRecvTask",
  .stack_size = 128 * 16,
  .priority = (osPriority_t) osPriorityNormal,
};


/* Message queues ------------------------------------------------------------*/
//Key message
// 定义消息队列
osMessageQueueId_t Key_MessageQueue;
osMessageQueueId_t Idle_MessageQueue;
osMessageQueueId_t Stop_MessageQueue;
osMessageQueueId_t IdleBreak_MessageQueue;
osMessageQueueId_t HomeUpdata_MessageQueue;
osMessageQueueId_t DataSave_MessageQueue;

/* Private function prototypes -----------------------------------------------*/
void LvHandlerTask(void *argument);
void WDOGFeedTask(void *argument);

/**
  * @brief  FreeRTOS initialization  创建自定义任务（核心）
  * @param  None
  * @retval None
  */
void User_Tasks_Init(void)
{
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */
  // 创建并启动 100ms 周期的空闲计时器（软件定时器）
	IdleTimerHandle = osTimerNew(IdleTimerCallback, osTimerPeriodic, NULL, NULL);
	osTimerStart(IdleTimerHandle, 100);//100ms

  /* add queues, ... */
  // 消息队列创建函数  队列长度、消息单元大小、描述消息队列的结构体
	Key_MessageQueue  = osMessageQueueNew(1, 1, NULL);
	Idle_MessageQueue = osMessageQueueNew(1, 1, NULL);
	Stop_MessageQueue = osMessageQueueNew(1, 1, NULL);
	IdleBreak_MessageQueue = osMessageQueueNew(1, 1, NULL);
	HomeUpdata_MessageQueue = osMessageQueueNew(1, 1, NULL);
	DataSave_MessageQueue = osMessageQueueNew(2, 1, NULL);

  // 串口控制台初始化
  UART_Console_Init();

	/* add threads, ... */
  // 使用osThreadNew函数创建自定义任务线程
  // 这里的每一个任务都应该是while(1)的loop形式
  HardwareInitTaskHandle  = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes); // 硬件初始化，自销毁

	IdleEnterTaskHandle  = osThreadNew(IdleEnterTask, NULL, &IdleEnterTask_attributes); // 屏保
	StopEnterTaskHandle  = osThreadNew(StopEnterTask, NULL, &StopEnterTask_attributes); // 休眠
	KeyTaskHandle 			 = osThreadNew(KeyTask, NULL, &KeyTask_attributes);  // 按键操作
	SensorDataTaskHandle = osThreadNew(SensorDataUpdateTask, NULL, &SensorDataTask_attributes);

	DataSaveTaskHandle		= osThreadNew(DataSaveTask, NULL, &DataSaveTask_attributes); // 保存数据线程

	ConsoleRecvTaskHandle = osThreadNew(ConsoleRecvTask, NULL, &ConsoleRecvTask_attributes); // 串口接收舵机控制

  /* add events, ... */


	/* add  others ... */
	uint8_t HomeUpdataStr;
  /*
  mq_id ：消息队列ID(消息队列句柄)
  msg_ptr ：发送消息的地址
  msg_prio ：发送优先级，在源码中可看到该参数被忽略并不生效。
  timeout ：线程等待时间
  */
	osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);

	// 添加测试模块的初始化
	FreeRTOS_Test_Init();
}


/**
  * @brief  FreeRTOS Tick Hook, to increase the LVGL tick
  * 这是 RTOS 的滴答钩子，每 1ms 执行一次
  * @param  None
  * @retval None
  */
void TaskTickHook(void)
{
	//to increase the timerpage's timer(put in here is to ensure the Real Time)
  // 处理秒表（TimerPage）的毫秒、秒、分累加逻辑
//	if(ui_TimerPageFlag)
//	{
//			ui_TimerPage_ms+=1;
//			if(ui_TimerPage_ms>=10)
//			{
//				ui_TimerPage_ms=0;
//				ui_TimerPage_10ms+=1;
//			}
//			if(ui_TimerPage_10ms>=100)
//			{
//					ui_TimerPage_10ms=0;
//					ui_TimerPage_sec+=1;
//					uint8_t IdleBreakstr = 0;
//					osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 0);
//			}
//			if(ui_TimerPage_sec>=60)
//			{
//					ui_TimerPage_sec=0;
//					ui_TimerPage_min+=1;
//			}
//			if(ui_TimerPage_min>=60)
//			{
//					ui_TimerPage_min=0;
//			}
//	}
//	user_HR_timecount+=1;

	  static uint32_t tick = 0;
	  static uint32_t d = 1;
	  tick+=d;

	  // 每 2000ms 切换一次位置
	  if (tick >= 1000) {
		  tick = 0;
		  d+=1;
	      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	  }

	  // 绝对不要在这里写任何 Delay!
	  static uint32_t direct_test_tick = 0;
	  direct_test_tick++;

	  // 每 500ms 切换一次位置
	  if (direct_test_tick >= 500) {
	      direct_test_tick = 0;
//	      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

	      life_count++;
		  // 喂狗也可以放在这里
		  HAL_IWDG_Refresh(&hiwdg1);
	  }


}




