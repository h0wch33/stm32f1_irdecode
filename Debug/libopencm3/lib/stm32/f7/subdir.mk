################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/stm32/f7/desig.c \
../libopencm3/lib/stm32/f7/flash.c \
../libopencm3/lib/stm32/f7/pwr.c \
../libopencm3/lib/stm32/f7/rcc.c \
../libopencm3/lib/stm32/f7/vector_chipset.c 

OBJS += \
./libopencm3/lib/stm32/f7/desig.o \
./libopencm3/lib/stm32/f7/flash.o \
./libopencm3/lib/stm32/f7/pwr.o \
./libopencm3/lib/stm32/f7/rcc.o \
./libopencm3/lib/stm32/f7/vector_chipset.o 

C_DEPS += \
./libopencm3/lib/stm32/f7/desig.d \
./libopencm3/lib/stm32/f7/flash.d \
./libopencm3/lib/stm32/f7/pwr.d \
./libopencm3/lib/stm32/f7/rcc.d \
./libopencm3/lib/stm32/f7/vector_chipset.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/stm32/f7/%.o: ../libopencm3/lib/stm32/f7/%.c libopencm3/lib/stm32/f7/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


