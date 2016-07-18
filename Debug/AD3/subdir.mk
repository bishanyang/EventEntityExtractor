################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AD3/Factor.cpp \
../AD3/FactorGraph.cpp \
../AD3/GenericFactor.cpp \
../AD3/Utils.cpp 

OBJS += \
./AD3/Factor.o \
./AD3/FactorGraph.o \
./AD3/GenericFactor.o \
./AD3/Utils.o 

CPP_DEPS += \
./AD3/Factor.d \
./AD3/FactorGraph.d \
./AD3/GenericFactor.d \
./AD3/Utils.d 


# Each subdirectory must supply rules for building sources it contributes
AD3/%.o: ../AD3/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


