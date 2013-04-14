################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/dataset.cpp \
../src/inference.cpp \
../src/main.cpp \
../src/math_func.cpp \
../src/model.cpp \
../src/polya_fit_simple.cpp \
../src/strtokenizer.cpp \
../src/utils.cpp 

OBJS += \
./src/dataset.o \
./src/inference.o \
./src/main.o \
./src/math_func.o \
./src/model.o \
./src/polya_fit_simple.o \
./src/strtokenizer.o \
./src/utils.o 

CPP_DEPS += \
./src/dataset.d \
./src/inference.d \
./src/main.d \
./src/math_func.d \
./src/model.d \
./src/polya_fit_simple.d \
./src/strtokenizer.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


