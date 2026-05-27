/**
  ******************************************************************************
  * @file    IAP_Main/Inc/flash_if.h 
  * @author  MCD Application Team
  * @brief   This file provides all the headers of the flash_if functions.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    BOOT_STATE_NONE  = 0,
    BOOT_STATE_BANKA = 1,
    BOOT_STATE_BANKB = 2
} BootState_t;

typedef struct {
    uint32_t magic;
    BootState_t boot_state;
    uint32_t vector_table_offset;
} BootInfo_t;
/* Exported constants --------------------------------------------------------*/
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0_BANK1     ((uint32_t)0x08000000) /* Base @ of Sector 0, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_1_BANK1     ((uint32_t)0x08020000) /* Base @ of Sector 1, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_2_BANK1     ((uint32_t)0x08040000) /* Base @ of Sector 2, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_3_BANK1     ((uint32_t)0x08060000) /* Base @ of Sector 3, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_4_BANK1     ((uint32_t)0x08080000) /* Base @ of Sector 4, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_5_BANK1     ((uint32_t)0x080A0000) /* Base @ of Sector 5, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6_BANK1     ((uint32_t)0x080C0000) /* Base @ of Sector 6, Bank1, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7_BANK1     ((uint32_t)0x080E0000) /* Base @ of Sector 7, Bank1, 128 Kbyte */

/* Flash layout:
   Sector 0 (0x08000000): Bootloader
   Sector 1 (0x08020000): BootInfo
   Sector 2 (0x08040000): SN
   Sector 3-4 (0x08060000-0x0809FFFF): BankA (256KB)
   Sector 5-6 (0x080A0000-0x080DFFFF): BankB (256KB)
   Sector 7 (0x080E0000): Reserved
*/
#define ADDR_BOOT_INFO          ADDR_FLASH_SECTOR_1_BANK1
#define ADDR_SN                 ADDR_FLASH_SECTOR_2_BANK1
#define ADDR_BANKA              ADDR_FLASH_SECTOR_3_BANK1
#define ADDR_BANKB              ADDR_FLASH_SECTOR_5_BANK1

#define BANK_FLASH_SIZE         ((uint32_t)0x00040000)
#define BANKA_END_ADDRESS       (ADDR_BANKA + BANK_FLASH_SIZE - 1)
#define BANKB_END_ADDRESS       (ADDR_BANKB + BANK_FLASH_SIZE - 1)

#define BOOT_INFO_SECTOR        FLASH_SECTOR_1
#define SN_SECTOR               FLASH_SECTOR_2
#define BANKA_START_SECTOR      FLASH_SECTOR_3
#define BANKA_END_SECTOR        FLASH_SECTOR_4
#define BANKB_START_SECTOR      FLASH_SECTOR_5
#define BANKB_END_SECTOR        FLASH_SECTOR_6

#define BOOT_INFO_MAGIC         ((uint32_t)0x424F4F54)

/* Error code */
enum 
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR
};
  
enum{
  FLASHIF_PROTECTION_NONE         = 0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED   = 0x2,
  FLASHIF_PROTECTION_RDPENABLED   = 0x4,
};

/* End of the Flash address */
#define USER_FLASH_END_ADDRESS        0x080FFFFF

#define APPLICATION_ADDRESS           ADDR_BANKA
#define USER_FLASH_SIZE               BANK_FLASH_SIZE

/* Define bitmap representing user flash area that could be write protected (check restricted to pages 8-39). */
#define FLASH_SECTOR_TO_BE_PROTECTED (OB_WRP_SECTOR_0 | OB_WRP_SECTOR_1 | OB_WRP_SECTOR_2 | OB_WRP_SECTOR_3 |\
                                      OB_WRP_SECTOR_4 | OB_WRP_SECTOR_5 | OB_WRP_SECTOR_6 | OB_WRP_SECTOR_7)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void              FLASH_If_Init(void);
uint32_t          FLASH_If_Erase(uint32_t StartSector);
uint32_t          FLASH_If_Erase_Sector(uint32_t Sector);
uint32_t          FLASH_If_Write(uint32_t FlashAddress, uint32_t* Data, uint32_t DataLength);
uint16_t          FLASH_If_GetWriteProtectionStatus(void);
HAL_StatusTypeDef FLASH_If_WriteProtectionConfig(uint32_t modifier);

BootInfo_t        BOOT_Info_Read(void);
uint32_t          BOOT_Info_Write(BootInfo_t *p_info);
uint32_t          FLASH_If_Erase_Bank(BootState_t bank);
uint32_t          BOOT_GetActiveAppAddress(void);
BootState_t       BOOT_GetInactiveBank(void);

#endif  /* __FLASH_IF_H */

