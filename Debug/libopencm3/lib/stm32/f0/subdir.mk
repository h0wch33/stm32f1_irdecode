################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/stm32/f0/adc.c \
../libopencm3/lib/stm32/f0/comparator.c \
../libopencm3/lib/stm32/f0/flash.c \
../libopencm3/lib/stm32/f0/i2c.c \
../libopencm3/lib/stm32/f0/rcc.c \
../libopencm3/lib/stm32/f0/syscfg.c 

OBJS += \
./libopencm3/lib/stm32/f0/adc.o \
./libopencm3/lib/stm32/f0/comparator.o \
./libopencm3/lib/stm32/f0/flash.o \
./libopencm3/lib/stm32/f0/i2c.o \
./libopencm3/lib/stm32/f0/rcc.o \
./libopencm3/lib/stm32/f0/syscfg.o 

C_DEPS += \
./libopencm3/lib/stm32/f0/adc.d \
./libopencm3/lib/stm32/f0/comparator.d \
./libopencm3/lib/stm32/f0/flash.d \
./libopencm3/lib/stm32/f0/i2c.d \
./libopencm3/lib/stm32/f0/rcc.d \
./libopencm3/lib/stm32/f0/syscfg.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/stm32/f0/%.o: ../libopencm3/lib/stm32/f0/%.c libopencm3/lib/stm32/f0/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


