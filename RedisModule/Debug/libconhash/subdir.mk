################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../libconhash/conhash.c \
../libconhash/conhash_inter.c \
../libconhash/conhash_util.c \
../libconhash/md5.c \
../libconhash/sample.c \
../libconhash/util_rbtree.c 

OBJS += \
./libconhash/conhash.o \
./libconhash/conhash_inter.o \
./libconhash/conhash_util.o \
./libconhash/md5.o \
./libconhash/sample.o \
./libconhash/util_rbtree.o 

C_DEPS += \
./libconhash/conhash.d \
./libconhash/conhash_inter.d \
./libconhash/conhash_util.d \
./libconhash/md5.d \
./libconhash/sample.d \
./libconhash/util_rbtree.d 


# Each subdirectory must supply rules for building sources it contributes
libconhash/%.o: ../libconhash/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


