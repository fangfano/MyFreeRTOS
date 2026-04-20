/* Private includes -----------------------------------------------------------*/
//includes
#include "user_DataSaveTask.h"

#include "main.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/******************************************
EEPROM Data description:
[0x00]:0x55 for check
[0x01]:0xAA for check

[0x10]:user wrist setting, HWInterface.IMU.wrist_is_enabled
[0x11]:user ui_APPSy_EN setting

[0x20]:Last Save Day(0-31)
[0x21]:Day Steps

*******************************************/


/* Private function prototypes -----------------------------------------------*/

/* Tasks ---------------------------------------------------------------------*/

/**
  * @brief  Data Save in the EEPROM
  * @param  argument: Not used
  * @retval None
  */
void DataSaveTask(void *argument)
{

	while(1)
	{
//		uint8_t Datastr=0;
//		// 从消息队列读取消息函数
//		// 当任务试图读队列中的消息时，可以指定一个阻塞超时时间，当且仅当消息队列中有消息的时候，任务才能读取到消息。
//		/*
//		mq_id ：消息队列ID(消息队列句柄)
//		msg_ptr ：接收消息的地址
//		msg_prio ：接收数据优先级，在源码中可看到该参数被忽略并不生效。
//		timeout ：线程等待时间
//		如果接收数据成功则返回osOK，失败返回各种失败代码
//		*/
//		// 保存传感器状态，包括温度、湿度、心率、步数
//		if(osMessageQueueGet(DataSave_MessageQueue,&Datastr,NULL,1)==osOK)
//		{
//			/****************
//			Setting change
//			date change
//			Step change
//			****************/
//			uint8_t dat[3];
//			dat[0] = HWInterface.IMU.wrist_is_enabled;
//			dat[1] = ui_APPSy_EN;
//			SettingSave(dat,0x10,2);
//
//			RTC_DateTypeDef nowdate;
//			HAL_RTC_GetDate(&hrtc,&nowdate,RTC_FORMAT_BIN);
//
//			SettingGet(dat,0x20,3);
//			if(dat[0] != nowdate.Date)
//			{
//				if(!HWInterface.IMU.ConnectionError)
//					HWInterface.IMU.SetSteps(0);
//
//				dat[0] = nowdate.Date;
//				dat[2] = 0;
//				dat[1] = 0;
//				SettingSave(dat,0x20,3);
//			}
//			else
//			{
//				uint16_t temp = HWInterface.IMU.GetSteps();
//				dat[0] = nowdate.Date;
//				dat[2] = temp & 0xff;
//				dat[1] = temp>>8 & 0xff;
//				SettingSave(dat,0x20,3);
//			}
//
//		}
		osDelay(100);
	}
}


