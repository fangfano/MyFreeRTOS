/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
#include "user_SensUpdateTask.h"

#include "main.h"


/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint32_t user_HR_timecount=0;

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  MPU6050 Check the state
  * 每 300ms 检查一次用户的动作，决定是否要让手表熄屏省电
  * @param  argument: Not used
  * @retval None
  */
void MPUCheckTask(void *argument)
{
	while(1)
	{
//		if(HWInterface.IMU.wrist_is_enabled)
//		{
//			// 调用 MPU_isHorizontal() 判断手表是否处于水平（看表）的位置
//			if(MPU_isHorizontal())
//			{
//				// 如果表平放了，状态设置为 WRIST_UP（抬腕）
//				HWInterface.IMU.wrist_state = WRIST_UP;
//			}
//			else
//			{
//				// 如果不是水平，且之前是抬腕，说明现在手放下去了
//				if(WRIST_UP == HWInterface.IMU.wrist_state)
//				{
//					// 状态设为 WRIST_DOWN（垂腕）
//					HWInterface.IMU.wrist_state = WRIST_DOWN;
//					// 如果你正处于主页、菜单页或设置页
//					if( Page_Get_NowPage()->page_obj == &ui_HomePage ||
//						Page_Get_NowPage()->page_obj == &ui_MenuPage ||
//						Page_Get_NowPage()->page_obj == &ui_SetPage )
//					{
//						uint8_t Stopstr;
//						// 进入休眠
//						osMessageQueuePut(Stop_MessageQueue, &Stopstr, 0, 1);//sleep
//					}
//				}
//				HWInterface.IMU.wrist_state = WRIST_DOWN;
//			}
//		}
		osDelay(300);
	}
}

/**
  * @brief  HR data renew task
  * 专门负责心率数据的实时计算
  * 
  * @param  argument: Not used
  * @retval None
  */
void HRDataUpdateTask(void *argument)
{
	uint8_t IdleBreakstr=0;
	uint16_t dat=0;
	uint8_t hr_temp=0;
	while(1)
	{
//		// 只有当你进入了 “心率页面” (ui_HRPage)，它才会真正开始工作
//		if(Page_Get_NowPage()->page_obj == &ui_HRPage)
//		{
//			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
//			//sensor wake up 调用 EM7028_hrs_Enable() 开启红外/绿光传感器
//			EM7028_hrs_Enable();
//			//receive the sensor wakeup message, sensor wakeup
//			if(!HWInterface.HR_meter.ConnectionError)
//			{
//				// 计算心率是非常耗时的算法逻辑，为了保证数据准确，计算时会暂时关闭任务调度
//				//Hr messure
//				vTaskSuspendAll();
//				hr_temp = HR_Calculate(EM7028_Get_HRS1(),user_HR_timecount);
//				xTaskResumeAll();
//				// 它会限制心率在 50 到 120 之间，防止因为手臂晃动产生的误差
//				if(HWInterface.HR_meter.HrRate != hr_temp && hr_temp>50 && hr_temp<120)
//				{
//					HWInterface.HR_meter.HrRate = hr_temp;
//				}
//			}
//		}
		osDelay(50);
	}
}


/**
  * @brief  Sensor data update task
  * 负责处理剩下的所有传感器，每 500ms 更新一次
  * @param  argument: Not used
  * @retval None
  */
void SensorDataUpdateTask(void *argument)
{
	uint8_t value_strbuf[6];
	uint8_t IdleBreakstr=0;
	while(1)
	{
//		// Update the sens data showed in Home
//		uint8_t HomeUpdataStr;
//		// 监听 HomeUpdata_MessageQueue 消息。一旦收到消息（通常是刚开机或刚从休眠醒来）
//		if(osMessageQueueGet(HomeUpdata_MessageQueue, &HomeUpdataStr, NULL, 0)==osOK)
//		{
//			//bat
//			uint8_t value_strbuf[5];
//			// 电量计算：更新剩余电量百分比
//			HWInterface.Power.power_remain = HWInterface.Power.BatCalculate();
//			if(HWInterface.Power.power_remain>0 && HWInterface.Power.power_remain<=100)
//			{}
//			else
//			{HWInterface.Power.power_remain = 0;}
//
//			//steps 计步更新：从传感器读取今日步数
//			if(!(HWInterface.IMU.ConnectionError))
//			{
//				HWInterface.IMU.Steps = HWInterface.IMU.GetSteps();
//			}
//
//			//temp and humi 环境温湿度：读取 AHT21 传感器的温度和湿度
//			if(!(HWInterface.AHT21.ConnectionError))
//			{
//				//temp and humi messure
//				float humi,temp;
//				HWInterface.AHT21.GetHumiTemp(&humi,&temp);
//				//check
//				if(temp>-10 && temp<50 && humi>0 && humi<100)
//				{
//					// ui_EnvTempValue = (int8_t)temp;
//					// ui_EnvHumiValue = (int8_t)humi;
//					HWInterface.AHT21.humidity = humi;
//					HWInterface.AHT21.temperature = temp;
//				}
//			}
//
//			//send data save message queue
//			uint8_t Datastr = 3;
//			// 完成后发送消息给 DataSave_MessageQueue，确保这些新数据被存入存储器
//			osMessageQueuePut(DataSave_MessageQueue, &Datastr, 0, 1);
//
//		}
//
//
//		// SPO2 Page
//		if(Page_Get_NowPage()->page_obj == &ui_SPO2Page)
//		{
//			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
//			//sensor wake up
//
//			//receive the sensor wakeup message, sensor wakeup
//			if(0)
//			{
//				//SPO2 messure
//			}
//		}
//		// Env Page 环境页 (ui_EnvPage)：高频率更新温湿度
//		else if(Page_Get_NowPage()->page_obj == &ui_EnvPage)
//		{
//			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
//			//receive the sensor wakeup message, sensor wakeup
//			if(!HWInterface.AHT21.ConnectionError)
//			{
//				//temp and humi messure
//				float humi,temp;
//				HWInterface.AHT21.GetHumiTemp(&humi,&temp);
//				//check
//				if(temp>-10 && temp<50 && humi>0 && humi<100)
//				{
//					HWInterface.AHT21.temperature = (int8_t)temp;
//					HWInterface.AHT21.humidity = (int8_t)humi;
//				}
//			}
//
//		}
//		// Compass page 指南针页 (ui_CompassPage) 唤醒 LSM303（磁力计）计算方位角（北在哪） 唤醒气压计计算海拔高度
//		else if(Page_Get_NowPage()->page_obj == &ui_CompassPage)
//		{
//			osMessageQueuePut(IdleBreak_MessageQueue, &IdleBreakstr, 0, 1);
//			//receive the sensor wakeup message, sensor wakeup
//			LSM303DLH_Wakeup();
//			//SPL_Wakeup();
//			//if the sensor is no problem
//			if(!HWInterface.Ecompass.ConnectionError)
//			{
//				//messure
//				int16_t Xa,Ya,Za,Xm,Ym,Zm;
//				LSM303_ReadAcceleration(&Xa,&Ya,&Za);
//				LSM303_ReadMagnetic(&Xm,&Ym,&Zm);
//				float temp = Azimuth_Calculate(Xa,Ya,Za,Xm,Ym,Zm)+0;//0 offset
//				if(temp<0)
//				{temp+=360;}
//				//check
//				if(temp>=0 && temp<=360)
//				{
//					HWInterface.Ecompass.direction = (uint16_t)temp;
//				}
//			}
//			//if the sensor is no problem
//			if(!HWInterface.Barometer.ConnectionError)
//			{
//				//messure
//				float alti = Altitude_Calculate();
//				//check
//				if(1)
//				{
//					HWInterface.Barometer.altitude = (int16_t)alti;
//				}
//			}
//		}

		osDelay(500);
	}
}
