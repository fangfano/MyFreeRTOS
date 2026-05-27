/**
  ******************************************************************************
  * @file    IAP_Main/Src/menu.c 
  * @author  MCD Application Team
  * @brief   This file provides the software which contains the main menu routine.
  *          The main menu gives the options of:
  *             - downloading a new binary file, 
  *             - uploading internal flash memory,
  *             - executing the binary file already loaded 
  *             - configuring the write protection of the Flash sectors where the 
  *               user loads his binary file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/** @addtogroup STM32H7xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "iwdg.h"
#include "common.h"
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"

#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction JumpToApplication;
uint32_t JumpAddress;
uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  带看门狗喂狗的串口字符串接收函数 (回车结束)
  */
static void Serial_ReceiveString(uint8_t *p_string, uint16_t max_length)
{
  uint16_t length = 0;
  uint8_t char_rx = 0;

  while (length < max_length - 1)
  {
    HAL_IWDG_Refresh(&hiwdg1); // 🐶 等待输入时必须不断喂狗

    if (HAL_UART_Receive(&UartHandle, &char_rx, 1, 10) == HAL_OK)
    {
      if (char_rx == '\r' || char_rx == '\n') // 按下回车键
      {
        if (length > 0) break; // 已经有输入内容则结束
      }
      else if (char_rx == '\b' || char_rx == 0x7F) // 按下退格键
      {
        if (length > 0)
        {
          length--;
          Serial_PutByte('\b');
          Serial_PutByte(' ');
          Serial_PutByte('\b');
        }
      }
      else if (char_rx >= 0x20 && char_rx <= 0x7E) // 限制为可见字符
      {
        p_string[length++] = char_rx;
        Serial_PutByte(char_rx); // 回显到终端
      }
    }
  }
  p_string[length] = '\0'; // 补全字符串结束符
  Serial_PutString((uint8_t *)"\r\n");
}

/**
  * @brief  Download a file via serial port (dual-bank OTA)
  */
void SerialDownload(void)
{
  uint8_t number[11] = {0};
  uint32_t size = 0;
  COM_StatusTypeDef result;
  BootInfo_t info;
  BootState_t target_bank;
  uint32_t target_addr;

  info = BOOT_Info_Read();
  target_bank = BOOT_GetInactiveBank();
  target_addr = (target_bank == BOOT_STATE_BANKA) ? ADDR_BANKA : ADDR_BANKB;

  Serial_PutString((uint8_t *)"\r\n--- OTA Dual-Bank Info ---\r\n");
  Serial_PutString((uint8_t *)"Active: ");
  Serial_PutString((info.boot_state == BOOT_STATE_BANKA) ? (uint8_t *)"BankA" :
                   (info.boot_state == BOOT_STATE_BANKB) ? (uint8_t *)"BankB" :
                   (uint8_t *)"None");
  Serial_PutString((uint8_t *)"\r\nTarget: ");
  Serial_PutString((target_bank == BOOT_STATE_BANKA) ? (uint8_t *)"BankA" : (uint8_t *)"BankB");
  Serial_PutString((uint8_t *)"\r\n---------------------------\r\n");

  Serial_PutString((uint8_t *)"Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  result = Ymodem_Receive(&size, target_addr, BANK_FLASH_SIZE);

  if (result == COM_OK)
  {
    BootInfo_t new_info;
    new_info.boot_state = target_bank;
    new_info.vector_table_offset = target_addr;

    if (BOOT_Info_Write(&new_info) == FLASHIF_OK)
    {
      Serial_PutString((uint8_t *)"\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
      Serial_PutString(aFileName);
      Int2Str(number, size);
      Serial_PutString((uint8_t *)"\n\r Size: ");
      Serial_PutString(number);
      Serial_PutString((uint8_t *)" Bytes\r\n");
      Serial_PutString((uint8_t *)"Flash address: 0x");
      Int2Hex(number, target_addr);
      Serial_PutString(number);
      Serial_PutString((uint8_t *)"\r\nActive bank switched to: ");
      Serial_PutString((target_bank == BOOT_STATE_BANKA) ? (uint8_t *)"BankA" : (uint8_t *)"BankB");
      Serial_PutString((uint8_t *)"\r\n-------------------\n");
    }
    else
    {
      Serial_PutString((uint8_t *)"\n\n\r Firmware written but BootInfo update FAILED!\n\r");
    }
  }

  else if (result == COM_LIMIT)
  {
    Serial_PutString((uint8_t *)"\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (result == COM_DATA)
  {
    Serial_PutString((uint8_t *)"\n\n\rVerification failed!\n\r");
  }
  else if (result == COM_ABORT)
  {
    Serial_PutString((uint8_t *)"\r\n\nAborted by user.\n\r");
  }
  else
  {
    Serial_PutString((uint8_t *)"\n\rFailed to receive the file!\n\r");
  }
}

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
void SerialUpload(void)
{
  uint8_t status = 0;

  Serial_PutString((uint8_t *)"\n\n\rSelect Receive File\n\r");

//  HAL_UART_Receive(&UartHandle, &status, 1, RX_TIMEOUT);
  // 🛑 核心修改：边等边喂狗
    status = 0;
    while (HAL_UART_Receive(&UartHandle, &status, 1, RX_TIMEOUT) != HAL_OK)
    {
        HAL_IWDG_Refresh(&hiwdg1); // 🐶 喂狗
    }

  if ( status == CRC16)
  {
    status = Ymodem_Transmit((uint8_t*)BOOT_GetActiveAppAddress(), (const uint8_t*)"UploadedFlashImage.bin", BANK_FLASH_SIZE);

    if (status != 0)
    {
      Serial_PutString((uint8_t *)"\n\rError Occurred while Transmitting File\n\r");
    }
    else
    {
      Serial_PutString((uint8_t *)"\n\rFile uploaded successfully \n\r");
    }
  }
}

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
  uint8_t key = 0;

  Serial_PutString((uint8_t *)"\r\n======================================================================");
  Serial_PutString((uint8_t *)"\r\n=              (C) COPYRIGHT 2017 STMicroelectronics                 =");
  Serial_PutString((uint8_t *)"\r\n=                                                                    =");
  Serial_PutString((uint8_t *)"\r\n=  STM32H7xx In-Application Programming Application  (Version 1.0.0) =");
  Serial_PutString((uint8_t *)"\r\n=                                                                    =");
  Serial_PutString((uint8_t *)"\r\n=                                   By MCD Application Team          =");
  Serial_PutString((uint8_t *)"\r\n======================================================================");
  Serial_PutString((uint8_t *)"\r\n\r\n");


  while (1)
  {
    uint8_t number[11] = {0};

	HAL_IWDG_Refresh(&hiwdg1);

    /* Test if any sector of Flash memory where user application will be loaded is write protected */
    FlashProtection = FLASH_If_GetWriteProtectionStatus();

    Serial_PutString((uint8_t *)"\r\n=================== Main Menu ============================\r\n\n");
    Serial_PutString((uint8_t *)"  Download image to inactive bank (OTA) --------------- 1\r\n\n");
    Serial_PutString((uint8_t *)"  Upload image from active bank ------------------------ 2\r\n\n");
    Serial_PutString((uint8_t *)"  Execute the loaded application ----------------------- 3\r\n\n");


    if(FlashProtection != FLASHIF_PROTECTION_NONE)
    {
      Serial_PutString((uint8_t *)"  Disable the write protection ------------------------- 4\r\n\n");
    }
    else
    {
      Serial_PutString((uint8_t *)"  Enable the write protection -------------------------- 4\r\n\n");
    }
    Serial_PutString((uint8_t *)"  Show boot info -------------------------------------- 5\r\n\n");
    Serial_PutString((uint8_t *)"  Query active bank ----------------------------------- 6\r\n\n");
    Serial_PutString((uint8_t *)"==========================================================\r\n\n");

    /* Clean the input path */
    __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
  
    /* Receive key */
    key = 0;
    while (HAL_UART_Receive(&UartHandle, &key, 1, RX_TIMEOUT) != HAL_OK)
    {
        HAL_IWDG_Refresh(&hiwdg1); // 必须在等待循环中喂狗
    }

    switch (key)
    {
      case '1' :
        /* Download user application in the Flash */
        SerialDownload();
        break;
      case '2' :
        /* Upload user application from the Flash */
        SerialUpload();
        break;
      case '3' :
        {
          uint32_t app_addr = BOOT_GetActiveAppAddress();
          if (app_addr == 0)
          {
            Serial_PutString((uint8_t *)"No valid application found in any bank!\r\n");
            break;
          }
          Serial_PutString((uint8_t *)"Start program execution from ");
          Serial_PutString((app_addr == ADDR_BANKA) ? (uint8_t *)"BankA" : (uint8_t *)"BankB");
          Serial_PutString((uint8_t *)"......\r\n\n");
          JumpAddress = *(__IO uint32_t*) (app_addr + 4);
          JumpToApplication = (pFunction) JumpAddress;
          __set_MSP(*(__IO uint32_t*) app_addr);
          JumpToApplication();
        }
        break;
      case '4' :
        if (FlashProtection != FLASHIF_PROTECTION_NONE)
        {
          /* Disable the write protection */
          if (FLASH_If_WriteProtectionConfig(DISABLE) == HAL_OK)
          {
            Serial_PutString((uint8_t *)"Write Protection disabled...\r\n");
            Serial_PutString((uint8_t *)"System will now restart...\r\n");
          }
          else
          {
            Serial_PutString((uint8_t *)"Error: Flash write un-protection failed...\r\n");
          }
        }
        else
        {
          if (FLASH_If_WriteProtectionConfig(ENABLE) == HAL_OK)
          {
            Serial_PutString((uint8_t *)"Write Protection enabled...\r\n");
            Serial_PutString((uint8_t *)"System will now restart...\r\n");
          }
          else
          {
            Serial_PutString((uint8_t *)"Error: Flash write protection failed...\r\n");
          }
        }
        break;
      case '5' :
        {
          BootInfo_t info = BOOT_Info_Read();
          Serial_PutString((uint8_t *)"\r\n--- Boot Info ---\r\n");
          Serial_PutString((uint8_t *)"State: ");
          Serial_PutString((info.boot_state == BOOT_STATE_BANKA) ? (uint8_t *)"BankA Active" :
                           (info.boot_state == BOOT_STATE_BANKB) ? (uint8_t *)"BankB Active" :
                           (uint8_t *)"No Program");
          Serial_PutString((uint8_t *)"\r\nVTOR: 0x");
          Int2Str(number, info.vector_table_offset);
          Serial_PutString(number);
          Serial_PutString((uint8_t *)"\r\nBankA SP: 0x");
          Int2Str(number, *(__IO uint32_t*)ADDR_BANKA);
          Serial_PutString(number);
          Serial_PutString((uint8_t *)"\r\nBankB SP: 0x");
          Int2Str(number, *(__IO uint32_t*)ADDR_BANKB);
          Serial_PutString(number);
          Serial_PutString((uint8_t *)"\r\n-----------------\r\n");
        }
        break;
      case '6' :
        {
          BootInfo_t info = BOOT_Info_Read();
          Serial_PutString((uint8_t *)"Active: ");
          Serial_PutString((info.boot_state == BOOT_STATE_BANKA) ? (uint8_t *)"BankA" :
                           (info.boot_state == BOOT_STATE_BANKB) ? (uint8_t *)"BankB" :
                           (uint8_t *)"None");
          Serial_PutString((uint8_t *)"\r\n");
        }
        break;
      default:
        Serial_PutString((uint8_t *)"Invalid Number ! ==> The number should be 1, 2, 3, 4, 5 or 6\r");
        break;
    }
  }
}

/**
  * @}
  */

