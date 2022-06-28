################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/stm32/f2/crypto.c \
../libopencm3/lib/stm32/f2/flash.c \
../libopencm3/lib/stm32/f2/i2c.c \
../libopencm3/lib/stm32/f2/rcc.c \
../libopencm3/lib/stm32/f2/rng.c 

OBJS += \
./libopencm3/lib/stm32/f2/crypto.o \
./libopencm3/lib/stm32/f2/flash.o \
./libopencm3/lib/stm32/f2/i2c.o \
./libopencm3/lib/stm32/f2/rcc.o \
./libopencm3/lib/stm32/f2/rng.o 

C_DEPS += \
./libopencm3/lib/stm32/f2/crypto.d \
./libopencm3/lib/stm32/f2/flash.d \
./libopencm3/lib/stm32/f2/i2c.d \
./libopencm3/lib/stm32/f2/rcc.d \
./libopencm3/lib/stm32/f2/rng.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/stm32/f2/%.o: ../libopencm3/lib/stm32/f2/%.c libopencm3/lib/stm32/f2/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


