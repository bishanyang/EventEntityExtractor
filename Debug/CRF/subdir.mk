################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CRF/encoder.cpp \
../CRF/feature.cpp \
../CRF/feature_cache.cpp \
../CRF/feature_index.cpp \
../CRF/lbfgs.cpp \
../CRF/node.cpp \
../CRF/param.cpp \
../CRF/tagger.cpp \
../CRF/tree_encoder.cpp \
../CRF/tree_node.cpp \
../CRF/tree_tagger.cpp \
../CRF/viterbi_decoder.cpp 

OBJS += \
./CRF/encoder.o \
./CRF/feature.o \
./CRF/feature_cache.o \
./CRF/feature_index.o \
./CRF/lbfgs.o \
./CRF/node.o \
./CRF/param.o \
./CRF/tagger.o \
./CRF/tree_encoder.o \
./CRF/tree_node.o \
./CRF/tree_tagger.o \
./CRF/viterbi_decoder.o 

CPP_DEPS += \
./CRF/encoder.d \
./CRF/feature.d \
./CRF/feature_cache.d \
./CRF/feature_index.d \
./CRF/lbfgs.d \
./CRF/node.d \
./CRF/param.d \
./CRF/tagger.d \
./CRF/tree_encoder.d \
./CRF/tree_node.d \
./CRF/tree_tagger.d \
./CRF/viterbi_decoder.d 


# Each subdirectory must supply rules for building sources it contributes
CRF/%.o: ../CRF/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


