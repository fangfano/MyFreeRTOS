#ifndef __USER_CONSOLERECVTASK_H__
#define __USER_CONSOLERECVTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SERVO_MIN_PULSE     500
#define SERVO_MAX_PULSE     2500
#define SERVO_COUNT         4

void ConsoleRecvTask(void *argument);
void Servo_SetPulse(uint8_t ch, uint16_t pulse);
uint16_t Servo_GetPulse(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif
