################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Tasks/Src/user_ConsoleRecvTask.c \
../User/Tasks/Src/user_DataSaveTask.c \
../User/Tasks/Src/user_FreeRTOS_Test.c \
../User/Tasks/Src/user_HardwareInitTask.c \
../User/Tasks/Src/user_KeyTask.c \
../User/Tasks/Src/user_RunModeTasks.c \
../User/Tasks/Src/user_SensUpdateTask.c \
../User/Tasks/Src/user_TasksInit.c 

OBJS += \
./User/Tasks/Src/user_ConsoleRecvTask.o \
./User/Tasks/Src/user_DataSaveTask.o \
./User/Tasks/Src/user_FreeRTOS_Test.o \
./User/Tasks/Src/user_HardwareInitTask.o \
./User/Tasks/Src/user_KeyTask.o \
./User/Tasks/Src/user_RunModeTasks.o \
./User/Tasks/Src/user_SensUpdateTask.o \
./User/Tasks/Src/user_TasksInit.o 

C_DEPS += \
./User/Tasks/Src/user_ConsoleRecvTask.d \
./User/Tasks/Src/user_DataSaveTask.d \
./User/Tasks/Src/user_FreeRTOS_Test.d \
./User/Tasks/Src/user_HardwareInitTask.d \
./User/Tasks/Src/user_KeyTask.d \
./User/Tasks/Src/user_RunModeTasks.d \
./User/Tasks/Src/user_SensUpdateTask.d \
./User/Tasks/Src/user_TasksInit.d 


# Each subdirectory must supply rules for building sources it contributes
User/Tasks/Src/%.o User/Tasks/Src/%.su User/Tasks/Src/%.cyclo: ../User/Tasks/Src/%.c User/Tasks/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Fano/Projects/brain_servo/brain_servo_canfd/User" -I"C:/Fano/Projects/brain_servo/brain_servo_canfd/User/Common/Inc" -I"C:/Fano/Projects/brain_servo/brain_servo_canfd/User/Tasks/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/RTOS2/Include -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Tasks-2f-Src

clean-User-2f-Tasks-2f-Src:
	-$(RM) ./User/Tasks/Src/user_ConsoleRecvTask.cyclo ./User/Tasks/Src/user_ConsoleRecvTask.d ./User/Tasks/Src/user_ConsoleRecvTask.o ./User/Tasks/Src/user_ConsoleRecvTask.su ./User/Tasks/Src/user_DataSaveTask.cyclo ./User/Tasks/Src/user_DataSaveTask.d ./User/Tasks/Src/user_DataSaveTask.o ./User/Tasks/Src/user_DataSaveTask.su ./User/Tasks/Src/user_FreeRTOS_Test.cyclo ./User/Tasks/Src/user_FreeRTOS_Test.d ./User/Tasks/Src/user_FreeRTOS_Test.o ./User/Tasks/Src/user_FreeRTOS_Test.su ./User/Tasks/Src/user_HardwareInitTask.cyclo ./User/Tasks/Src/user_HardwareInitTask.d ./User/Tasks/Src/user_HardwareInitTask.o ./User/Tasks/Src/user_HardwareInitTask.su ./User/Tasks/Src/user_KeyTask.cyclo ./User/Tasks/Src/user_KeyTask.d ./User/Tasks/Src/user_KeyTask.o ./User/Tasks/Src/user_KeyTask.su ./User/Tasks/Src/user_RunModeTasks.cyclo ./User/Tasks/Src/user_RunModeTasks.d ./User/Tasks/Src/user_RunModeTasks.o ./User/Tasks/Src/user_RunModeTasks.su ./User/Tasks/Src/user_SensUpdateTask.cyclo ./User/Tasks/Src/user_SensUpdateTask.d ./User/Tasks/Src/user_SensUpdateTask.o ./User/Tasks/Src/user_SensUpdateTask.su ./User/Tasks/Src/user_TasksInit.cyclo ./User/Tasks/Src/user_TasksInit.d ./User/Tasks/Src/user_TasksInit.o ./User/Tasks/Src/user_TasksInit.su

.PHONY: clean-User-2f-Tasks-2f-Src

