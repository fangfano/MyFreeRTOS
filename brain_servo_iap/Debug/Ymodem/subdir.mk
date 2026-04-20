################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Ymodem/common.c \
../Ymodem/flash_if.c \
../Ymodem/menu.c \
../Ymodem/ymodem.c 

OBJS += \
./Ymodem/common.o \
./Ymodem/flash_if.o \
./Ymodem/menu.o \
./Ymodem/ymodem.o 

C_DEPS += \
./Ymodem/common.d \
./Ymodem/flash_if.d \
./Ymodem/menu.d \
./Ymodem/ymodem.d 


# Each subdirectory must supply rules for building sources it contributes
Ymodem/%.o Ymodem/%.su Ymodem/%.cyclo: ../Ymodem/%.c Ymodem/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I"C:/Fano/Projects/brain_servo/brain_servo_iap/Ymodem" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Ymodem

clean-Ymodem:
	-$(RM) ./Ymodem/common.cyclo ./Ymodem/common.d ./Ymodem/common.o ./Ymodem/common.su ./Ymodem/flash_if.cyclo ./Ymodem/flash_if.d ./Ymodem/flash_if.o ./Ymodem/flash_if.su ./Ymodem/menu.cyclo ./Ymodem/menu.d ./Ymodem/menu.o ./Ymodem/menu.su ./Ymodem/ymodem.cyclo ./Ymodem/ymodem.d ./Ymodem/ymodem.o ./Ymodem/ymodem.su

.PHONY: clean-Ymodem

