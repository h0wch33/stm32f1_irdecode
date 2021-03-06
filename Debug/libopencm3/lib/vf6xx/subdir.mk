################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libopencm3/lib/vf6xx/ccm.c \
../libopencm3/lib/vf6xx/gpio.c \
../libopencm3/lib/vf6xx/iomuxc.c \
../libopencm3/lib/vf6xx/uart.c \
../libopencm3/lib/vf6xx/vector_chipset.c 

OBJS += \
./libopencm3/lib/vf6xx/ccm.o \
./libopencm3/lib/vf6xx/gpio.o \
./libopencm3/lib/vf6xx/iomuxc.o \
./libopencm3/lib/vf6xx/uart.o \
./libopencm3/lib/vf6xx/vector_chipset.o 

C_DEPS += \
./libopencm3/lib/vf6xx/ccm.d \
./libopencm3/lib/vf6xx/gpio.d \
./libopencm3/lib/vf6xx/iomuxc.d \
./libopencm3/lib/vf6xx/uart.d \
./libopencm3/lib/vf6xx/vector_chipset.d 


# Each subdirectory must supply rules for building sources it contributes
libopencm3/lib/vf6xx/%.o: ../libopencm3/lib/vf6xx/%.c libopencm3/lib/vf6xx/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DSTM32F1 -DUSE_FULL_ASSERT -DSTM32F103x6 -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I/home/src/eclipse-workspace/stm32f1_dfu/libopencm3/include -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


