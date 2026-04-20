/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "iwdg.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "menu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t boot_in_menu_flag = 0;

extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
void Jump_To_App(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_IWDG1_Init();
  /* USER CODE BEGIN 2 */
  __HAL_DBGMCU_FREEZE_IWDG1(); // 调试时冻结看门狗

	uint8_t enter_iap = 0;
	uint32_t life_count = 0;

	// 1. 软件触发判定：检查 RTC 备份寄存器（由 APP 重启触发）
	// 注意：需要确保前面已经调用了 MX_RTC_Init(); 否则这里可能读不到
	HAL_PWR_EnableBkUpAccess();
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) == 0x5A5A) {
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x0000); // 擦除标志
		enter_iap = 1;
	}

//	while(1){ // 喂狗与生命状态测试
//		HAL_IWDG_Refresh(&hiwdg1); // 必须在等待循环中喂狗
//		life_count++;
//		printf("\r\n[BOOT] Test life %ld\r\n", life_count);
//	    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
//	    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
//		HAL_Delay(500);
//	}

	HAL_IWDG_Refresh(&hiwdg1); // 喂狗

	// 2. UART 触发判定：PC 串口拦截（替换原来的按键逻辑）
	if (enter_iap == 0) {
		printf("\r\n[BOOT] Press 'U' within 3 seconds to force enter IAP mode...\r\n");

		uint32_t tickstart = HAL_GetTick();
		uint8_t rx_data = 0;

		// 开启 3 秒 (3000ms) 的等待窗口
		while ((HAL_GetTick() - tickstart) < 3000) {

			// 极其重要：等待期间必须持续喂狗，否则单片机会在此处不断复位！
			HAL_IWDG_Refresh(&hiwdg1);

			// 尝试通过轮询方式接收 1 个字节，超时时间设为 10ms，保证循环能快速运转去喂狗
			if (HAL_UART_Receive(&huart1, &rx_data, 1, 10) == HAL_OK) {
				// 如果收到了特定的标识符（例如大写字母 'U'）
				if (rx_data == 'U') {
					printf("\r\n[BOOT] Intercepted! Entering IAP mode...\r\n");
					enter_iap = 1;
					break; // 拦截成功，跳出等待循环
				}
			}
		}
	}

	// 3. 进入 IAP 模式
	if (enter_iap) {
		FLASH_If_Init(); // 解锁 Flash 权限
		Main_Menu();     // 进入 Ymodem 菜单循环
	}
	// 4. 尝试跳转到 APP
	else {
		// 直接引入你的 APP 起始地址宏 (在 flash_if.h 里定义的那个)
		// 如果没 include，就直接写死 uint32_t app_addr = 0x08020000;
		uint32_t app_addr = APPLICATION_ADDRESS;

		// 工业标准做法：检查 APP 地址的第一个字（栈顶指针）是否指向 RAM 区域
		// STM32H7 的 RAM 通常在 0x20000000 (DTCM) 或 0x24000000 (AXI SRAM)
		uint32_t app_stack_ptr = *(__IO uint32_t*)app_addr;

		if ((app_stack_ptr & 0x2FF00000) == 0x20000000 ||
			(app_stack_ptr & 0x2FF00000) == 0x24000000)
		{
			printf("[BOOT] Legal APP found (Valid SP: 0x%08X), jumping...\r\n", app_stack_ptr);
			HAL_Delay(50); // 稍微延时一下让串口把字打印完
			Jump_To_App(); // 执行跳转
		}
		else {
			// 如果读出来的栈顶指针是 0xFFFFFFFF (空Flash)，或者其他乱七八糟的数字，说明没程序
			printf("[BOOT] Error: No legal APP found at 0x%08X!\r\n", app_addr);
			printf("[BOOT] Read Stack Pointer: 0x%08X\r\n", app_stack_ptr);
			HAL_Delay(2000);

			// 如果没有 APP，强制进入 IAP 菜单以便用户下载固件
			FLASH_If_Init();
			Main_Menu();
		}
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		printf("run in boot while(1)\r\n");
    printf("there is no legal APP\r\n");
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	  HAL_Delay(2000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 15;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void Jump_To_App(void)
{
    uint32_t app_addr = APPLICATION_ADDRESS;
    uint32_t app_stack_ptr = *(__IO uint32_t*)app_addr;

    if (((app_stack_ptr & 0x2FF00000) == 0x20000000) ||
        ((app_stack_ptr & 0x2FF00000) == 0x24000000))
    {
        // 1. 关闭全局中断
        __disable_irq();

        printf("1\n");

        // 临走前狠狠喂一次狗
        extern IWDG_HandleTypeDef hiwdg1;
        HAL_IWDG_Refresh(&hiwdg1);

        // 如果你的 Bootloader 用到了 ADC DMA，必须在此强制停止！
		// extern ADC_HandleTypeDef hadc1;
		// HAL_ADC_Stop_DMA(&hadc1);

		// 强制停止串口的所有收发和 DMA
		extern UART_HandleTypeDef huart1; // 根据你代码里的名字，可能是 huart1
		HAL_UART_Abort(&huart1);

        printf("2\n");
		// 强制插入同步屏障，确保前面的关闭动作在硬件层面上彻底完成
		// 如果有隐藏的 BusFault，也会在这里提前引爆，而不会死在 Cache 函数里
		__DSB();
		__ISB();

        // 2. 【先】清理并关闭 Cache
        // 此时总线和内存时钟都在正常运行，写回脏数据绝对安全
        SCB_DisableICache();
//        SCB_CleanInvalidateDCache();
//        SCB_DisableDCache();
        SCB_InvalidateDCache();
        SCB->CCR &= ~(uint32_t)SCB_CCR_DC_Msk; // 强制从硬件位关闭 D-Cache

        printf("3\n");
		__DSB();
		__ISB();

        // 3. 【再】关闭 MPU
        HAL_MPU_Disable();

        // 4. 【最后】反初始化外设
        // 现在 Cache 已经关了，怎么关外设时钟都不会触发总线错误了
        HAL_DeInit();
        //printf("4\n");
        // ==========================================

        // 5. 禁用 SysTick
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        // 6. 清理残留的中断标志
        for (int i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
        //printf("5\n");

        // 7. 执行跳转
        uint32_t JumpAddress = *(__IO uint32_t*) (app_addr + 4);
        pFunction JumpToApplication = (pFunction) JumpAddress;

        // 设置主堆栈指针
        __set_MSP(app_stack_ptr);

        // 冲！
        //printf("\r\n[BOOT] All cleared, JUMPING NOW!\r\n");

        JumpToApplication();
    }
    else
    {
        printf("Error: Stack Pointer not valid!\r\n");
    }
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	// 打印遗言，这样你就知道是不是时钟配置失败导致的重启了！
	  printf("\r\n[BOOT] Crashed in Error_Handler!\r\n");
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
