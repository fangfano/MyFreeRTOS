#ifndef __USER_RUNMODETASKS_H__
#define __USER_RUNMODETASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "user_TasksInit.h"
#include "stdio.h"

#include "main.h"
#include "stm32h7xx_it.h"
#include "usart.h"

void IdleEnterTask(void *argument);
void StopEnterTask(void *argument);
void IdleTimerCallback(void *argument);

extern uint16_t IdleTimerCount;

#ifdef __cplusplus
}
#endif

#endif

