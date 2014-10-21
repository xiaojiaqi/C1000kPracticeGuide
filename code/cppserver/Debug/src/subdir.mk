################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/client.cc \
../src/network.cc \
../src/worker.cc 

CC_SRCS += \
../src/listenfd.cc \
../src/main.cc \
../src/main_event.cc \
../src/socket_util.cc \
../src/timers.cc 

OBJS += \
./src/client.o \
./src/listenfd.o \
./src/main.o \
./src/main_event.o \
./src/network.o \
./src/socket_util.o \
./src/timers.o \
./src/worker.o 

CC_DEPS += \
./src/listenfd.d \
./src/main.d \
./src/main_event.d \
./src/socket_util.d \
./src/timers.d 

CPP_DEPS += \
./src/client.d \
./src/network.d \
./src/worker.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


