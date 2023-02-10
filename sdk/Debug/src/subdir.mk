################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/Fixed.c \
../src/Layer.c \
../src/TinyYoloCtrl.c \
../src/Video.c \
../src/Weight.c \
../src/WeightFileConverter.c \
../src/dma.c \
../src/image.c \
../src/lps_vdma.c \
../src/main.c 

OBJS += \
./src/Fixed.o \
./src/Layer.o \
./src/TinyYoloCtrl.o \
./src/Video.o \
./src/Weight.o \
./src/WeightFileConverter.o \
./src/dma.o \
./src/image.o \
./src/lps_vdma.o \
./src/main.o 

C_DEPS += \
./src/Fixed.d \
./src/Layer.d \
./src/TinyYoloCtrl.d \
./src/Video.d \
./src/Weight.d \
./src/WeightFileConverter.d \
./src/dma.d \
./src/image.d \
./src/lps_vdma.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../standalone_bsp_0/ps7_cortexa9_0/include -I../inc -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


