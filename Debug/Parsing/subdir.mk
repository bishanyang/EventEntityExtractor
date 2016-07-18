################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Parsing/GraphNode.cpp \
../Parsing/PennTree.cpp \
../Parsing/TreeNode.cpp 

OBJS += \
./Parsing/GraphNode.o \
./Parsing/PennTree.o \
./Parsing/TreeNode.o 

CPP_DEPS += \
./Parsing/GraphNode.d \
./Parsing/PennTree.d \
./Parsing/TreeNode.d 


# Each subdirectory must supply rules for building sources it contributes
Parsing/%.o: ../Parsing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


