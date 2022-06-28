################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/stm32/f3/adc.c \
../libopencm3/lib/stm32/f3/flash.c \
../libopencm3/lib/stm32/f3/i2c.c \
../libopencm3/lib/stm32/f3/rcc.c \
../libopencm3/lib/stm32/f3/vector_chipset.c 

OBJS += \
./libopencm3/lib/stm32/f3/adc.o \
./libopencm3/lib/stm32/f3/flash.o \
./libopencm3/lib/stm32/f3/i2c.o \
./libopencm3/lib/stm32/f3/rcc.o \
./libopencm3/lib/stm32/f3/vector_chipset.o 

C_DEPS += \
./libopencm3/lib/stm32/f3/adc.d \
./libopencm3/lib/stm32/f3/flash.d \
./libopencm3/lib/stm32/f3/i2c.d \
./libopencm3/lib/stm32/f3/rcc.d \
./libopencm3/lib/stm32/f3/vector_chipset.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/stm32/f3/%.o: ../libopencm3/lib/stm32/f3/%.c libopencm3/lib/stm32/f3/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


