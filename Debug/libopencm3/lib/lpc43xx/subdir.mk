################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/lpc43xx/gpio.c \
../libopencm3/lib/lpc43xx/i2c.c \
../libopencm3/lib/lpc43xx/ipc.c \
../libopencm3/lib/lpc43xx/scu.c \
../libopencm3/lib/lpc43xx/ssp.c \
../libopencm3/lib/lpc43xx/timer.c \
../libopencm3/lib/lpc43xx/uart.c 

OBJS += \
./libopencm3/lib/lpc43xx/gpio.o \
./libopencm3/lib/lpc43xx/i2c.o \
./libopencm3/lib/lpc43xx/ipc.o \
./libopencm3/lib/lpc43xx/scu.o \
./libopencm3/lib/lpc43xx/ssp.o \
./libopencm3/lib/lpc43xx/timer.o \
./libopencm3/lib/lpc43xx/uart.o 

C_DEPS += \
./libopencm3/lib/lpc43xx/gpio.d \
./libopencm3/lib/lpc43xx/i2c.d \
./libopencm3/lib/lpc43xx/ipc.d \
./libopencm3/lib/lpc43xx/scu.d \
./libopencm3/lib/lpc43xx/ssp.d \
./libopencm3/lib/lpc43xx/timer.d \
./libopencm3/lib/lpc43xx/uart.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/lpc43xx/%.o: ../libopencm3/lib/lpc43xx/%.c libopencm3/lib/lpc43xx/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


