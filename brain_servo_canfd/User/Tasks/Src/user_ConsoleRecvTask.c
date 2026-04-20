#include "user_ConsoleRecvTask.h"
#include "uart_console.h"
#include "tim.h"
#include "user_TasksInit.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rtc.h"

static uint16_t servo_pulse[SERVO_COUNT] = {1500, 1500, 1500, 1500};


void Servo_SetPulse(uint8_t ch, uint16_t pulse)
{
    if (ch < 1 || ch > SERVO_COUNT) return;
    if (pulse < SERVO_MIN_PULSE) pulse = SERVO_MIN_PULSE;
    if (pulse > SERVO_MAX_PULSE) pulse = SERVO_MAX_PULSE;
    
    servo_pulse[ch - 1] = pulse;
    
    switch (ch) {
        case 1: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse); break;
        case 2: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse); break;
        case 3: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pulse); break;
        case 4: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, pulse); break;
    }
}

uint16_t Servo_GetPulse(uint8_t ch)
{
    if (ch < 1 || ch > SERVO_COUNT) return 0;
    return servo_pulse[ch - 1];
}

static void ParseCommand(uint8_t *data, uint16_t len)
{
    // ==========================================
    // 1. 优先拦截 OTA 升级命令 (假设发送的是 "OTA")
    // ==========================================
    if (len >= 3 && strncmp((char *)data, "OTA", 3) == 0) {
        printf("\r\n[APP] OTA trigger received! Rebooting to Bootloader...\r\n");
        osDelay(100); // 稍微延时，等待串口把这句话打印完 (FreeRTOS中用osDelay)

        // 解锁备份域权限
        HAL_PWR_EnableBkUpAccess();

        // 往信箱里写入我们在 Bootloader 约定的暗号：0x5A5A
        // 注意：APP工程的CubeMX里也必须开启 RTC 外设，否则没有 hrtc
        extern RTC_HandleTypeDef hrtc;
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x5A5A);

        // 斩立决：执行系统软复位！
        // 复位后，单片机会纯净地从 Bootloader 启动，读取暗号并停留在 IAP 界面
        NVIC_SystemReset();

        while(1); // 代码永远不会跑到这里
    }

    // ==========================================
    // 2. 正常的舵机控制命令解析 (保留你原有的逻辑)
    // ==========================================
    if (len < 4 || data[0] != 'S') return;
    
    uint8_t ch = data[1] - '0';
    if (ch < 1 || ch > SERVO_COUNT) {
        printf("ERR:CH\r\n");
        return;
    }
    
    if (data[2] != '=') {
        printf("ERR:FMT\r\n");
        return;
    }
    
    int pulse = atoi((char *)&data[3]);
    if (pulse < SERVO_MIN_PULSE || pulse > SERVO_MAX_PULSE) {
        printf("ERR:RANGE\r\n");
        return;
    }
    
    Servo_SetPulse(ch, (uint16_t)pulse);

    printf("OK:S%d=%d | ALL_STATUS: [S1:%d S2:%d S3:%d S4:%d]\r\n",
           ch, pulse,
           servo_pulse[0], servo_pulse[1], servo_pulse[2], servo_pulse[3]);
}

void ConsoleRecvTask(void *argument)
{
    __HAL_TIM_MOE_ENABLE(&htim1);
    
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    
    Servo_SetPulse(1, 1500);
    Servo_SetPulse(2, 1500);
    Servo_SetPulse(3, 1500);
    Servo_SetPulse(4, 1500);
    
    // ================== 修改部分开始 ==================
    // 启动时的提示也加上当前的初始值反馈
    printf("Servo Ready 1 2 3. CMD: S<1-4>=<500-2500> | INIT_STATUS: [S1:%d S2:%d S3:%d S4:%d]\r\n",
           servo_pulse[0], servo_pulse[1], servo_pulse[2], servo_pulse[3]);
    // ================== 修改部分结束 ==================
    
    UartRxMsg_t rx_msg;
	while (1) {
		// 等待来自 UART_Console 的消息
		if (osMessageQueueGet(UartRx_MessageQueue, &rx_msg, NULL, osWaitForever) == osOK) {
			ParseCommand(rx_msg.data, rx_msg.len);
		}
	}
}
