#ifndef __SN_MANAGER_H
#define __SN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#define SN_FLASH_ADDRESS    0x08040000
#define SN_MAX_LENGTH       512

#define SN_OK               0
#define SN_ERROR            1
#define SN_INVALID_FORMAT   2
#define SN_EMPTY            3

void SN_Init(void);
void SN_Print(void);
uint8_t SN_Read(char *sn_buffer, uint16_t max_len);
uint8_t SN_Write(const char *sn_data, uint16_t len);
uint8_t SN_ParseCommand(const uint8_t *data, uint16_t len);
void VERSION_Print(void);

#ifdef __cplusplus
}
#endif

#endif