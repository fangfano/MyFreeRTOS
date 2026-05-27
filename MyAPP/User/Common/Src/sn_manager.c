#include "sn_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"
#include "iwdg.h"
#include "version.h"

static uint32_t SN_GetSector(uint32_t Address);
static uint8_t SN_EraseSector(void);
static uint8_t SN_ProgramFlash(uint32_t Address, uint8_t *Data, uint16_t Len);

void SN_Init(void)
{
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGSERR);
}

void SN_Print(void)
{
    char *sn_buffer = (char *)malloc(SN_MAX_LENGTH + 1);
    if (sn_buffer == NULL)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)"[APP] SN: [Malloc Error]\r\n", 27, 100);
        return;
    }
    memset(sn_buffer, 0, SN_MAX_LENGTH + 1);

    uint8_t res = SN_Read(sn_buffer, SN_MAX_LENGTH);

    if (res == SN_OK)
    {
        uint16_t out_len = strlen(sn_buffer) + 16;
        char *out_str = (char *)malloc(out_len);
        if (out_str)
        {
            sprintf(out_str, "[APP] SN: %s\r\n", sn_buffer);
            HAL_UART_Transmit(&huart1, (uint8_t *)out_str, strlen(out_str), out_len);
            free(out_str);
        }
    }
    else if (res == SN_EMPTY)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)"[APP] SN: [Empty]\r\n", 19, 100);
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)"[APP] SN: [Read Error]\r\n", 25, 100);
    }

    free(sn_buffer);
}

uint8_t SN_Read(char *sn_buffer, uint16_t max_len)
{
    uint8_t *flash_ptr = (uint8_t *)SN_FLASH_ADDRESS;

    if (sn_buffer == NULL || max_len == 0) return SN_ERROR;

    if (flash_ptr[0] == 0xFF || flash_ptr[0] == '\0')
    {
        sn_buffer[0] = '\0';
        return SN_EMPTY;
    }

    uint16_t i;
    for (i = 0; i < max_len - 1 && i < SN_MAX_LENGTH; i++)
    {
        sn_buffer[i] = flash_ptr[i];
        if (sn_buffer[i] == '\0') break;
    }
    sn_buffer[i] = '\0';

    return SN_OK;
}

uint8_t SN_Write(const char *sn_data, uint16_t len)
{
    if (sn_data == NULL || len == 0 || len > SN_MAX_LENGTH) return SN_ERROR;

    uint16_t total_len = len + 1;
    uint16_t aligned_len = (total_len + 31) & ~31;

    uint8_t *write_buffer = (uint8_t *)malloc(aligned_len);
    if (write_buffer == NULL) return SN_ERROR;

    memset(write_buffer, 0xFF, aligned_len);
    memcpy(write_buffer, sn_data, len);
    write_buffer[len] = '\0';

    __disable_irq();

    if (SN_EraseSector() != SN_OK)
    {
        free(write_buffer);
        __enable_irq();
        return SN_ERROR;
    }

    uint16_t offset;
    for (offset = 0; offset < aligned_len; offset += 32)
    {
        if (SN_ProgramFlash(SN_FLASH_ADDRESS + offset, write_buffer + offset, 32) != SN_OK)
        {
            free(write_buffer);
            __enable_irq();
            return SN_ERROR;
        }
    }

    free(write_buffer);
    __enable_irq();

    return SN_OK;
}

uint8_t SN_ParseCommand(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len < 3) return SN_INVALID_FORMAT;

    if (strncmp((const char *)data, "SN=", 3) == 0)
    {
        const char *sn_start = (const char *)data + 3;
        uint16_t sn_len = len - 3;

        while (sn_len > 0 && (sn_start[sn_len - 1] == '\r' ||
                              sn_start[sn_len - 1] == '\n' ||
                              sn_start[sn_len - 1] == ' '))
        {
            sn_len--;
        }

        if (sn_len == 0 || sn_len > SN_MAX_LENGTH)
        {
            printf("[APP] SN Write Failed: Invalid length (max %d)!\r\n", SN_MAX_LENGTH);
            return SN_INVALID_FORMAT;
        }

        char *sn_buffer = (char *)malloc(sn_len + 1);
        if (sn_buffer == NULL)
        {
            printf("[APP] SN Write Failed: Malloc error!\r\n");
            return SN_ERROR;
        }
        memcpy(sn_buffer, sn_start, sn_len);
        sn_buffer[sn_len] = '\0';

        uint8_t result = SN_Write(sn_buffer, sn_len);

        if (result == SN_OK)
        {
            printf("[APP] SN Write Success: %s\r\n", sn_buffer);
        }
        else
        {
            printf("[APP] SN Write Failed!\r\n");
        }

        free(sn_buffer);
        return result;
    }

    return SN_INVALID_FORMAT;
}

void VERSION_Print(void)
{
    char out_str[64];
    sprintf(out_str, "[APP] Version: %d.%d.%d\r\n",
            VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    HAL_UART_Transmit(&huart1, (uint8_t *)out_str, strlen(out_str), 64);
}

static uint32_t SN_GetSector(uint32_t Address)
{
    uint32_t sector = 0;
    if (Address < 0x08020000) sector = 0;
    else if (Address < 0x08040000) sector = 1;
    else if (Address < 0x08060000) sector = 2;
    else if (Address < 0x08080000) sector = 3;
    else if (Address < 0x080A0000) sector = 4;
    else if (Address < 0x080C0000) sector = 5;
    else if (Address < 0x080E0000) sector = 6;
    else sector = 7;
    return sector;
}

#if defined(__ICCARM__)
__ramfunc
#elif defined(__CC_ARM) || defined(__GNUC__)
__attribute__((section(".RamFunc")))
#endif
static uint8_t SN_EraseSector_RAM(uint32_t Sector)
{
    while ((FLASH->SR1 & 0x01) != 0) {
        IWDG1->KR = 0xAAAA;
    }

    uint32_t cr_val = FLASH->CR1;
    cr_val &= ~(0x7U << 8);
    cr_val &= ~(0x3U << 4);
    cr_val &= ~(0x1U << 2);

    cr_val |= (Sector << 8);
    cr_val |= (0x2U << 4);
    cr_val |= (0x1U << 2);
    FLASH->CR1 = cr_val;

    FLASH->CR1 |= (0x1U << 7);

    while ((FLASH->SR1 & 0x01) != 0) {
        IWDG1->KR = 0xAAAA;
    }

    FLASH->CR1 &= ~(0x1U << 2);
    return SN_OK;
}

static uint8_t SN_EraseSector(void)
{
    uint32_t sector = SN_GetSector(SN_FLASH_ADDRESS);

    HAL_FLASH_Unlock();
    SCB_DisableICache();

    uint8_t res = SN_EraseSector_RAM(sector);

    SCB_EnableICache();
    HAL_FLASH_Lock();

    return res;
}

static uint8_t SN_ProgramFlash(uint32_t Address, uint8_t *Data, uint16_t Len)
{
    HAL_FLASH_Unlock();
    SCB_DisableICache();

    uint32_t i;
    for (i = 0; i < Len; i += 32)
    {
        HAL_IWDG_Refresh(&hiwdg1);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, Address + i,
                              (uint32_t)((uint32_t)Data + i)) != HAL_OK)
        {
            SCB_EnableICache();
            HAL_FLASH_Lock();
            return SN_ERROR;
        }
    }

    SCB_EnableICache();
    HAL_FLASH_Lock();

    return SN_OK;
}