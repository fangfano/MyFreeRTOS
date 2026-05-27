#ifndef __UART_CONSOLE_H__
#define __UART_CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os2.h"

#define UART_RX_BUF_SIZE    128

typedef struct {
    uint8_t len;
    uint8_t data[UART_RX_BUF_SIZE];
} UartRxMsg_t;

extern osMessageQueueId_t UartRx_MessageQueue;
extern osMutexId_t UartTx_Mutex;


void UART_Console_Init(void);
void UART_Console_Send(const uint8_t *data, uint16_t len);
void UART_RxIdleCallback(void);

#ifdef __cplusplus
}
#endif

#endif
