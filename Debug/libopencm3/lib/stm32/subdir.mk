################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/stm32/can.c \
../libopencm3/lib/stm32/st_usbfs_v1.c \
../libopencm3/lib/stm32/st_usbfs_v2.c 

OBJS += \
./libopencm3/lib/stm32/can.o \
./libopencm3/lib/stm32/st_usbfs_v1.o \
./libopencm3/lib/stm32/st_usbfs_v2.o 

C_DEPS += \
./libopencm3/lib/stm32/can.d \
./libopencm3/lib/stm32/st_usbfs_v1.d \
./libopencm3/lib/stm32/st_usbfs_v2.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/stm32/%.o: ../libopencm3/lib/stm32/%.c libopencm3/lib/stm32/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


