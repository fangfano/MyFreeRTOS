/* Private includes -----------------------------------------------------------*/
//includes
#include "user_RunModeTasks.h"


/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// 这是一个“空闲计数器”
uint16_t IdleTimerCount = 0;

/* Private function prototypes -----------------------------------------------*/

/* Tasks ---------------------------------------------------------------------*/

/**
	* @brief  Enter Idle state
	屏幕亮度管理任务
	根据消息切换屏幕亮度
  * @param  argument: Not used
  * @retval None
  */
void IdleEnterTask(void *argument)
{
	uint8_t Idlestr=0;
	uint8_t IdleBreakstr=0;
	while(1)
	{
//		// light get dark
//		// 变暗： 监听到 Idle_MessageQueue（空闲消息）
//		if(osMessageQueueGet(Idle_MessageQueue,&Idlestr,NULL,1)==osOK)
//		{
//			LCD_Set_Light(5); // 把亮度调到极低（暗屏），但没完全关掉
//		}
//		// resume light if light got dark and idle state breaked by key pressing or screen touching
//		// 恢复亮屏： 监听到 IdleBreak_MessageQueue（唤醒消息，通常由按键或触摸触发）
//		if(osMessageQueueGet(IdleBreak_MessageQueue,&IdleBreakstr,NULL,1)==osOK)
//		{
//			IdleTimerCount = 0;
//			// 恢复到用户设定的正常亮度
//			LCD_Set_Light(ui_LightSliderValue);
//		}
		// printf("life count: %ld\n", life_count);
		osDelay(5000);
	}
}

/**
  * @brief  enter the stop mode and resume
  * 深度休眠任务
  * 负责让芯片进入 STOP 模式（极低功耗模式）
  * @param  argument: Not used
  * @retval None
  */
void StopEnterTask(void *argument)
{
	uint8_t Stopstr;
	uint8_t HomeUpdataStr;
	uint8_t Wrist_Flag=0;
	while(1)
	{
//		if(osMessageQueueGet(Stop_MessageQueue,&Stopstr,NULL,0)==osOK)
//		{
//
//			/*************************** your operations before sleep***************************/
//			sleep:
//			IdleTimerCount = 0;
//
//			// 把串口、屏幕背光、触摸芯片全部关掉或降功耗
//			//sensors
//
//			//usart
//			HAL_UART_MspDeInit(&huart1);
//
//			//lcd
//			LCD_RES_Clr();
//			LCD_Close_Light();
//			//touch
//			CST816_Sleep();
//
//			/***********************************************************************************/
//
//			/****************************** enter wakeup operations *****************************/
//			// 暂停所有 RTOS 任务调度
//			vTaskSuspendAll();
//			// 关掉看门狗（不然睡觉时没喂狗，芯片会重启）
//			//Disnable Watch Dog
//			WDOG_Disnable();
//			// 关掉系统滴答定时器（心跳信号），这是进入深睡眠的前提
//			//systick int
//			CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
//			// 核心命令，CPU 停止工作，进入真正的低功耗状态。此时代码停在这里，直到被外部中断（按键、闹钟、充电、翻腕）唤醒
//			// enter stop mode
//			HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI);
//
//			//here is the sleep period
//
//			/***********************************************************************************/
//
//			/****************************** quit wakeup operations *****************************/
//			// 醒来恢复
//			//resume run mode and reset the sysclk
//			SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
//			HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
//			// 醒来第一件事是恢复时钟。刚醒时主频很乱，需要重新配置。
//			SystemClock_Config();
//			WDOG_Feed();
//			// 恢复所有任务运行
//			xTaskResumeAll();
//
//			/***********************************************************************************/
//
//			/****************************** your wakeup operations *****************************/
//			// 检查为什么醒来
//			//MPU Check 翻腕检查
//			if(HWInterface.IMU.wrist_is_enabled)
//			{
//				uint8_t hor;
//				hor = MPU_isHorizontal();
//				// 如果是翻腕亮屏，但现在手又放下去了，代码会执行 goto sleep 接着睡，不浪费电
//				if(hor && HWInterface.IMU.wrist_state == WRIST_DOWN)
//				{
//					HWInterface.IMU.wrist_state = WRIST_UP;
//					Wrist_Flag = 1;
//					//resume, go on
//				}
//				// 如果是按键按下或插上充电器，则继续
//				else if(!hor && HWInterface.IMU.wrist_state == WRIST_UP)
//				{
//					HWInterface.IMU.wrist_state = WRIST_DOWN;
//					IdleTimerCount  = 0;
//					goto sleep;
//				}
//			}
//
//			//
//			if(!KEY1 || KEY2 || HardInt_Charg_flag || Wrist_Flag)
//			{
//				Wrist_Flag = 0;
//				//resume, go on
//			}
//			else
//			{
//				IdleTimerCount  = 0;
//				goto sleep;
//			}
//
//			// 重新初始化外设
//			// usart
//			HAL_UART_MspInit(&huart1);
//			// lcd
//			LCD_Init();
//			LCD_Set_Light(ui_LightSliderValue);
//			// touch
//			CST816_Wakeup();
//			// check if is Charging
//			if(ChargeCheck())
//			{HardInt_Charg_flag = 1;}
//			// send the Home Updata message
//			osMessageQueuePut(HomeUpdata_MessageQueue, &HomeUpdataStr, 0, 1);
//
//			/**************************************************************************************/
//
//		}
		osDelay(100);
	}
}

// 空闲计时回调，软件定时器，每100ms执行一次， 是整个流程的“发令员”
void IdleTimerCallback(void *argument)
{
//	IdleTimerCount+=1;
//	// 到达变暗时间
//	//make sure the LightOffTime<TurnOffTime
//	if(IdleTimerCount == (ui_LTimeValue*10))
//	{
//		uint8_t Idlestr=0;
//		//send the Light off message
//		osMessageQueuePut(Idle_MessageQueue, &Idlestr, 0, 1);
//
//	}
//	// 到达休眠时间
//	if(IdleTimerCount == (ui_TTimeValue*10))
//	{
//		uint8_t Stopstr = 1;
//		IdleTimerCount  = 0;
//		//send the Stop message
//		osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);
//	}
//	osDelay(10);
}


