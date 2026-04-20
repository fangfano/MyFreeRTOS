################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Common/Src/Define.c \
../User/Common/Src/uart_console.c 

OBJS += \
./User/Common/Src/Define.o \
./User/Common/Src/uart_console.o 

C_DEPS += \
./User/Common/Src/Define.d \
./User/Common/Src/uart_console.d 


# Each subdirectory must supply rules for building sources it contributes
User/Common/Src/%.o User/Common/Src/%.su User/Common/Src/%.cyclo: ../User/Common/Src/%.c User/Common/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Fano/Projects/brain_servo/brain_servo_canfd/User" -I"C:/Fano/Projects/brain_servo/brain_servo_canfd/User/Common/Inc" -I"C:/Fano/Projects/brain_servo/brain_servo_canfd/User/Tasks/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/RTOS2/Include -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Common-2f-Src

clean-User-2f-Common-2f-Src:
	-$(RM) ./User/Common/Src/Define.cyclo ./User/Common/Src/Define.d ./User/Common/Src/Define.o ./User/Common/Src/Define.su ./User/Common/Src/uart_console.cyclo ./User/Common/Src/uart_console.d ./User/Common/Src/uart_console.o ./User/Common/Src/uart_console.su

.PHONY: clean-User-2f-Common-2f-Src

