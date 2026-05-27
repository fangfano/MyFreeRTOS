/* Private includes -----------------------------------------------------------*/
// 该任务在系统启动时运行一次，完成所有外设初始化后自我销毁

// includes
// sys
#include "usart.h"
#include "tim.h"
#include "stm32h7xx_it.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

// user
#include "user_TasksInit.h"
#include "version.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  hardwares init task
  * @param  argument: Not used
  * @retval None
  */
void HardwareInitTask(void *argument)
{
    // 这里做所有BSP硬件外设的初始化，保留在这里
	while(1)
	{
      // 开始初始化前挂起所有任务，确保初始化不被干扰
//      vTaskSuspendAll();

//    // 启动实时时钟（RTC）的定时唤醒。这能让手表在休眠时每隔 2 秒醒来检查一下
//    if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2000, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
//    {
//      Error_Handler();
//    }
//    // usart start 开启串口的 DMA（直接存储器访问） 接收
//    HAL_UART_Receive_DMA(&huart1,(uint8_t*)HardInt_receive_str,25);
//    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
//
//    // PWM Start 可能用于背光或马达
//    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
//
//    // sys delay 初始化
//    delay_init();
//    // wait
//    // delay_ms(1000);
//
//    // power
//    HWInterface.Power.Init();
//
//    // key
//    Key_Port_Init();
//
//
//    // sensors
//    // 传感器循环初始化
//    // 给传感器 3 次机会。如果初始化失败（ConnectionError），就再试一次
//    // 温湿度 (AHT21)
//    uint8_t num = 3;
//    while(num && HWInterface.AHT21.ConnectionError)
//    {
//      num--;
//      HWInterface.AHT21.ConnectionError = HWInterface.AHT21.Init();
//    }
//
//    // 电子罗盘 (Ecompass)
//    num = 3;
//    while(num && HWInterface.Ecompass.ConnectionError)
//    {
//      num--;
//      HWInterface.Ecompass.ConnectionError = HWInterface.Ecompass.Init();
//    }
//    if(!HWInterface.Ecompass.ConnectionError)
//      HWInterface.Ecompass.Sleep();
//
//    // 气压计 (Barometer)
//    num = 3;
//    while(num && HWInterface.Barometer.ConnectionError)
//    {
//      num--;
//      HWInterface.Barometer.ConnectionError = HWInterface.Barometer.Init();
//    }
//
//    // 运动传感器 (IMU)
//    num = 3;
//    while(num && HWInterface.IMU.ConnectionError)
//    {
//      num--;
//      HWInterface.IMU.ConnectionError = HWInterface.IMU.Init();
//      // Sensor_MPU_Erro = MPU_Init();
//    }
//
//    // 心率计 (HR_meter)
//    num = 3;
//    while(num && HWInterface.HR_meter.ConnectionError)
//    {
//      num--;
//      HWInterface.HR_meter.ConnectionError = HWInterface.HR_meter.Init();
//    }
//    if(!HWInterface.HR_meter.ConnectionError)
//      HWInterface.HR_meter.Sleep();
//
//
//    // EEPROM
//    EEPROM_Init();
//    // 检查存储芯片
//    if(!EEPROM_Check())
//    {
//      uint8_t recbuf[3];
//      // 从存储器里读取你之前的设置（比如有没有开启“抬腕亮屏”）
//      SettingGet(recbuf,0x10,2);
//      if((recbuf[0]!=0 && recbuf[0]!=1) || (recbuf[1]!=0 && recbuf[1]!=1))
//      {
//        HWInterface.IMU.wrist_is_enabled = 0;
//        ui_APPSy_EN = 0;
//      }
//      else
//      {
//        HWInterface.IMU.wrist_is_enabled = recbuf[0];
//        ui_APPSy_EN = recbuf[1];
//      }
//
//      RTC_DateTypeDef nowdate;
//      HAL_RTC_GetDate(&hrtc,&nowdate,RTC_FORMAT_BIN);
//
//      // 记步
//      // 步数恢复：它会对比今天的日期。如果日期没变，就把昨天存的步数读出来接力，防止重启后步数归零
//      SettingGet(recbuf,0x20,3);
//      if(recbuf[0] == nowdate.Date)
//      {
//        uint16_t steps=0;
//        steps = recbuf[1]&0x00ff;
//        steps = steps<<8 | recbuf[2];
//        if(!HWInterface.IMU.ConnectionError)
//          dmp_set_pedometer_step_count((unsigned long)steps);
//      }
//    }
//
//
//    // BLE 初始化
//    HWInterface.BLE.Init();
//    HWInterface.BLE.Disable();
//
//    //set the KT6328 BautRate 9600
//    //default is 115200
//    //printf("AT+CT01\r\n");
//      HAL_Delay(10);

    // 恢复之前挂起的所有任务，让系统恢复并发运行
//    xTaskResumeAll();
    // 最关键的一行。这个任务的任务已经完成了（硬件全开了），所以它把自己从任务列表中彻底删除
      osThreadExit(); // 完美退场：终止并清理当前正在运行的线程
    // 虽然任务已删，这行基本跑不到，但作为规范通常会留个小延迟
		osDelay(500);
	}
}


