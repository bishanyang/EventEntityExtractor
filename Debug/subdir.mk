################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AD3Inference.cpp \
../CoNLLDocumentReader.cpp \
../Dataset.cpp \
../Document.cpp \
../FeatureDict.cpp \
../Logger.cpp \
../Sentence.cpp \
../SequenceModel.cpp \
../TreeCRFModel.cpp \
../Utils.cpp \
../event_extractor.cpp 

OBJS += \
./AD3Inference.o \
./CoNLLDocumentReader.o \
./Dataset.o \
./Document.o \
./FeatureDict.o \
./Logger.o \
./Sentence.o \
./SequenceModel.o \
./TreeCRFModel.o \
./Utils.o \
./event_extractor.o 

CPP_DEPS += \
./AD3Inference.d \
./CoNLLDocumentReader.d \
./Dataset.d \
./Document.d \
./FeatureDict.d \
./Logger.d \
./Sentence.d \
./SequenceModel.d \
./TreeCRFModel.d \
./Utils.d \
./event_extractor.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


