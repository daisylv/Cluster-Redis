################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ClusterModule/cluster.c \
../ClusterModule/tests.c 

OBJS += \
./ClusterModule/cluster.o \
./ClusterModule/tests.o 

C_DEPS += \
./ClusterModule/cluster.d \
./ClusterModule/tests.d 


# Each subdirectory must supply rules for building sources it contributes
ClusterModule/%.o: ../ClusterModule/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


