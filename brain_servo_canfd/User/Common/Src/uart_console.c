#include "uart_console.h"
#include "usart.h"
#include "cmsis_os2.h"
#include "user_ConsoleRecvTask.h" // 获取 UartRxMsg_t 定义
#include <stdio.h>
#include <string.h>

// 1. 同步对象：互斥锁保证发送不冲突，信号量用于任务与DMA硬件同步
static osMutexId_t UartTxMutex;
static osSemaphoreId_t UartTxSem;
// 定义消息队列
osMessageQueueId_t UartRx_MessageQueue;

// 2. 缓冲区：必须是全局/静态的，DMA 才能访问
#define UART_BUF_SIZE 512
static uint8_t g_tx_buf[UART_BUF_SIZE];
static uint8_t g_rx_buf[UART_BUF_SIZE];



void UART_Console_Init(void) {
    UartTxMutex = osMutexNew(NULL);
    UartTxSem = osSemaphoreNew(1, 0, NULL); // 初始值为0，发完后再释放

	// 创建一个能存 5 条消息的接收队列。消息大小参考你在 ConsoleRecvTask 里用的 UartRxMsg_t
	UartRx_MessageQueue = osMessageQueueNew(5, sizeof(UartRxMsg_t), NULL);

	// 必须加上这句！启动第一次空闲中断 DMA 接收
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, g_rx_buf, UART_BUF_SIZE);
}

// 3. 重定向 printf (带锁的 DMA 发送)
int _write(int file, char *ptr, int len) {
    if (len > UART_BUF_SIZE) len = UART_BUF_SIZE;

    // 获取互斥锁，防止多任务抢占串口
    if (osMutexAcquire(UartTxMutex, osWaitForever) == osOK) {
        memcpy(g_tx_buf, ptr, len);

        // H7 必须清理 D-Cache，否则 DMA 发出的可能是旧数据
        SCB_CleanDCache_by_Addr((uint32_t *)g_tx_buf, len);

        if (HAL_UART_Transmit_DMA(&huart1, g_tx_buf, len) == HAL_OK) {
            // 等待 DMA 发送完成（由回调函数释放信号量）
            osSemaphoreAcquire(UartTxSem, osWaitForever);
        }
        osMutexRelease(UartTxMutex);
    }
    return len;
}

// 4. 发送完成回调
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        osSemaphoreRelease(UartTxSem);
    }
}

// 5. 接收完成/空闲回调
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance == USART1) {
        // H7 必须失效 D-Cache，确保 CPU 能读到 DMA 搬运的新数据
        SCB_InvalidateDCache_by_Addr((uint32_t *)g_rx_buf, Size);

        UartRxMsg_t msg;
        msg.len = (Size > sizeof(msg.data)) ? sizeof(msg.data) : Size;
        memcpy(msg.data, g_rx_buf, msg.len);

        // 推入队列（中断内调用 timeout 必须为 0）
        osMessageQueuePut(UartRx_MessageQueue, &msg, 0, 0);

        // 重新开启接收
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, g_rx_buf, UART_BUF_SIZE);
    }
}
