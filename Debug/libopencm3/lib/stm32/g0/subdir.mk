################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/stm32/g0/adc.c \
../libopencm3/lib/stm32/g0/flash.c \
../libopencm3/lib/stm32/g0/pwr.c \
../libopencm3/lib/stm32/g0/rcc.c 

OBJS += \
./libopencm3/lib/stm32/g0/adc.o \
./libopencm3/lib/stm32/g0/flash.o \
./libopencm3/lib/stm32/g0/pwr.o \
./libopencm3/lib/stm32/g0/rcc.o 

C_DEPS += \
./libopencm3/lib/stm32/g0/adc.d \
./libopencm3/lib/stm32/g0/flash.d \
./libopencm3/lib/stm32/g0/pwr.d \
./libopencm3/lib/stm32/g0/rcc.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/stm32/g0/%.o: ../libopencm3/lib/stm32/g0/%.c libopencm3/lib/stm32/g0/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


