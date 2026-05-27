/**
  ******************************************************************************
  * @file    IAP_Main/Src/flash_if.c 
  * @author  MCD Application Team
  * @brief   This file provides all the memory related operation functions.
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
#include "flash_if.h"
#include "iwdg.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(uint32_t Address);

/* Private functions ---------------------------------------------------------*/

/* ========================================================================= */
/* 🌟 核心：在 RAM 中运行的擦除操作，配合外部的 __disable_irq() 杜绝死锁 */
/* ========================================================================= */
#if defined(__ICCARM__)
__ramfunc
#elif defined(__CC_ARM) || defined(__GNUC__)
__attribute__((section(".RamFunc")))
#endif
static uint32_t FLASH_EraseSector_RAM(uint32_t Sector)
{
    /* 等待前一次 Flash 操作完成 */
    while ((FLASH->SR1 & 0x01) != 0) {
        IWDG1->KR = 0xAAAA;
    }

    uint32_t cr_val = FLASH->CR1;
    cr_val &= ~(0x7U << 8);  /* 清除 SNB */
    cr_val |= (Sector << 8); /* 设置要擦除的扇区 */
    cr_val &= ~(0x3U << 4);  /* 清除 PSIZE */
    cr_val |= (0x2U << 4);   /* PSIZE = 32-bit */
    cr_val |= (0x1U << 2);   /* 开启 SER */
    FLASH->CR1 = cr_val;

    /* 启动擦除 */
    FLASH->CR1 |= (0x1U << 7);

    /* 等待擦除结束并疯狂喂狗 */
    while ((FLASH->SR1 & 0x01) != 0) {
        IWDG1->KR = 0xAAAA;
    }

    /* 擦除完成，关闭 SER */
    FLASH->CR1 &= ~(0x1U << 2);
    return FLASHIF_OK;
}

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{
  
  /* Clear pending flags (if any) */  
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                         FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
}


/**
  * @brief  安全擦除应用程序 Flash 区域
  */
uint32_t FLASH_If_Erase(uint32_t StartSectorAddress)
{
  uint32_t UserStartSector;

  HAL_FLASH_Unlock(); 
  FLASH_If_Init();
  SCB_DisableICache();
  
  UserStartSector = GetSector(StartSectorAddress);
  
  // 🌟 核心防死机：关中断，进入纯净环境，防止任何中断跳回 Flash
  __disable_irq();

  for (uint32_t i = UserStartSector; i < 8; i++)
  {
      FLASH_EraseSector_RAM(i);
  }

  // 🌟 擦除完毕，恢复系统中断
  __enable_irq();

  SCB_EnableICache();
  HAL_FLASH_Lock();
  return FLASHIF_OK;
}

/**
  * @brief  安全擦除单个指定扇区
  */
uint32_t FLASH_If_Erase_Sector(uint32_t Sector)
{
  HAL_FLASH_Unlock();
  FLASH_If_Init();
  SCB_DisableICache();

  // 🌟 核心防死机
  __disable_irq();
  FLASH_EraseSector_RAM(Sector);
  __enable_irq();

  SCB_EnableICache();
  HAL_FLASH_Lock(); 
  return FLASHIF_OK;
}

/**
  * @brief  安全写入数据到 Flash
  */
uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  uint32_t i = 0;
  HAL_StatusTypeDef write_status;

  HAL_FLASH_Unlock();
  SCB_DisableICache();

  for (i = 0; (i < DataLength) && (FlashAddress <= (USER_FLASH_END_ADDRESS-32)); i+=8)
  {
    HAL_IWDG_Refresh(&hiwdg1); // 每写一次前喂狗

    // 🌟 短暂关闭中断：虽然写入单行很快，但防止就在此刻被中断打断引起总线读取竞争
    __disable_irq();
    write_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FlashAddress, ((uint32_t)(Data+i)));
    __enable_irq();

    if (write_status == HAL_OK)
    {
      if (*(uint32_t*)FlashAddress != *(uint32_t*)(Data+i))
      {
        SCB_EnableICache();
        HAL_FLASH_Lock();
        return FLASHIF_WRITINGCTRL_ERROR;
      }
      FlashAddress += 32;
    }
    else
    {
      SCB_EnableICache();
      HAL_FLASH_Lock();
      return FLASHIF_WRITING_ERROR;
    }
  }

  SCB_EnableICache();
  HAL_FLASH_Lock();
  return FLASHIF_OK;
}

/**
  * @brief  Returns the write protection status of user flash area.
  * @param  None
  * @retval 0: No write protected sectors inside the user flash area
  *         1: Some sectors inside the user flash area are write protected
  */
uint16_t FLASH_If_GetWriteProtectionStatus(void)
{
  uint32_t ProtectedSECTOR = 0x0;
  FLASH_OBProgramInitTypeDef OptionsBytesStruct;

  OptionsBytesStruct.Banks = FLASH_BANK_1;
  HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);
  ProtectedSECTOR = OptionsBytesStruct.WRPSector & FLASH_SECTOR_TO_BE_PROTECTED;

  if(ProtectedSECTOR != 0)
    return FLASHIF_PROTECTION_WRPENABLED;
  else
    return FLASHIF_PROTECTION_NONE;
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address: Flash address
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1))
  {
    sector = FLASH_SECTOR_6;  
  }
  else /*if((Address < USER_FLASH_END_ADDRESS) && (Address >= ADDR_FLASH_SECTOR_7_BANK2))*/
  {
    sector = FLASH_SECTOR_7;  
  }

  return sector;
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  modifier DISABLE or ENABLE the protection
  * @retval HAL_StatusTypeDef HAL_OK if change is applied.
  */
HAL_StatusTypeDef FLASH_If_WriteProtectionConfig(uint32_t modifier)
{
  uint32_t ProtectedSECTOR = 0xFFF;
  FLASH_OBProgramInitTypeDef config_new, config_old;
  HAL_StatusTypeDef result = HAL_OK;
  
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();
  
  /* ✅ 强制只操作 Bank1 */
  config_old.Banks = FLASH_BANK_1;
  config_new.Banks = FLASH_BANK_1;
  
  HAL_FLASHEx_OBGetConfig(&config_old);
  config_new.WRPState = modifier;
  config_new.OptionType = OPTIONBYTE_WRP;
  config_new.RDPLevel = OB_RDP_LEVEL_0;
  config_new.USERConfig = config_old.USERConfig;  
  
  ProtectedSECTOR = config_old.WRPSector | FLASH_SECTOR_TO_BE_PROTECTED;
  config_new.WRPSector = ProtectedSECTOR;
  
  result = HAL_FLASHEx_OBProgram(&config_new);
  
  HAL_FLASH_OB_Launch();
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();
  
  return result;
}

/**
  * @}
  */

BootInfo_t BOOT_Info_Read(void)
{
    BootInfo_t *p_flash_info = (BootInfo_t *)ADDR_BOOT_INFO;
    BootInfo_t info;

    info = *p_flash_info;

    if (info.magic != BOOT_INFO_MAGIC ||
        info.vector_table_offset != ((info.boot_state == BOOT_STATE_BANKA) ? ADDR_BANKA :
                                     (info.boot_state == BOOT_STATE_BANKB) ? ADDR_BANKB : 0))
    {
        info.magic = 0;
        info.boot_state = BOOT_STATE_NONE;
        info.vector_table_offset = 0;
    }

    return info;
}

uint32_t BOOT_Info_Write(BootInfo_t *p_info)
{
    uint32_t result;
    uint8_t write_buf[32];
    uint32_t i;

    p_info->magic = BOOT_INFO_MAGIC;
    p_info->vector_table_offset = (p_info->boot_state == BOOT_STATE_BANKA) ? ADDR_BANKA :
                                  (p_info->boot_state == BOOT_STATE_BANKB) ? ADDR_BANKB : 0;

    for (i = 0; i < sizeof(BootInfo_t) && i < 32; i++)
    {
        write_buf[i] = ((uint8_t *)p_info)[i];
    }
    for (; i < 32; i++)
    {
        write_buf[i] = 0xFF;
    }

    result = FLASH_If_Erase_Sector(BOOT_INFO_SECTOR);
    if (result != FLASHIF_OK)
    {
        return result;
    }

    return FLASH_If_Write(ADDR_BOOT_INFO, (uint32_t *)write_buf, 8);
}

uint32_t FLASH_If_Erase_Bank(BootState_t bank)
{
    uint32_t start_sector, end_sector;
    uint32_t result;

    if (bank == BOOT_STATE_BANKA)
    {
        start_sector = BANKA_START_SECTOR;
        end_sector = BANKA_END_SECTOR;
    }
    else if (bank == BOOT_STATE_BANKB)
    {
        start_sector = BANKB_START_SECTOR;
        end_sector = BANKB_END_SECTOR;
    }
    else
    {
        return FLASHIF_ERASEKO;
    }

    for (uint32_t s = start_sector; s <= end_sector; s++)
    {
        result = FLASH_If_Erase_Sector(s);
        if (result != FLASHIF_OK)
        {
            return result;
        }
    }

    return FLASHIF_OK;
}

static uint8_t IsAppValid(uint32_t app_addr)
{
    uint32_t sp = *(__IO uint32_t *)app_addr;
    return ((sp & 0x2FF00000) == 0x20000000 || (sp & 0x2FF00000) == 0x24000000);
}

uint32_t BOOT_GetActiveAppAddress(void)
{
    BootInfo_t info = BOOT_Info_Read();

    if (info.boot_state == BOOT_STATE_BANKA && IsAppValid(ADDR_BANKA))
    {
        return ADDR_BANKA;
    }
    else if (info.boot_state == BOOT_STATE_BANKB && IsAppValid(ADDR_BANKB))
    {
        return ADDR_BANKB;
    }
    else if (info.boot_state == BOOT_STATE_NONE)
    {
        if (IsAppValid(ADDR_BANKA)) return ADDR_BANKA;
        if (IsAppValid(ADDR_BANKB)) return ADDR_BANKB;
        return 0;
    }

    if (IsAppValid(ADDR_BANKA)) return ADDR_BANKA;
    if (IsAppValid(ADDR_BANKB)) return ADDR_BANKB;
    return 0;
}

BootState_t BOOT_GetInactiveBank(void)
{
    BootInfo_t info = BOOT_Info_Read();

    if (info.boot_state == BOOT_STATE_BANKA)
    {
        return BOOT_STATE_BANKB;
    }
    else if (info.boot_state == BOOT_STATE_BANKB)
    {
        return BOOT_STATE_BANKA;
    }

    return BOOT_STATE_BANKA;
}

