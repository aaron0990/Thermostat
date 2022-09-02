################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O4 --opt_for_speed=1 --include_path="C:/ti/ccs1110/ccs/workspace/Thermostat" --include_path="C:/ti/ccs1110/ccs/workspace/Thermostat/Release" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source/third_party/CMSIS/Include" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source/ti/posix/ccs" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/FreeRTOS/FreeRTOS/Source/include" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/FreeRTOS/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/ccs1110/ccs/workspace/freertos_builds_MSP_EXP432P401R_release_ccs" --include_path="C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/include" --include_path="C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source/ti/devices/msp432p4xx/driverlib" --advice:power="all" --advice:power_severity=warning --define=BOARD_DISPLAY_USE_UART_ANSI=1 --define=BOARD_DISPLAY_USE_LCD=1 --define=DeviceFamily_MSP432P401x --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/ti/ccs1110/ccs/workspace/Thermostat/Release/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


