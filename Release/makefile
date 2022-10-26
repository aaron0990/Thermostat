################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./DS3231.obj" \
"./DS3231_I2C.obj" \
"./DisplayConsole.obj" \
"./Keypad.obj" \
"./LCDdisplayClient.obj" \
"./LCDdisplayI2C.obj" \
"./LCDdisplayProxy.obj" \
"./MSP_EXP432P401R.obj" \
"./RTC.obj" \
"./TempController.obj" \
"./TempData.obj" \
"./TempSensor.obj" \
"./TempSensorProxy.obj" \
"./mainThread.obj" \
"./main_freertos.obj" \
"./utils.obj" \
"../MSP_EXP432P401R_FREERTOS.cmd" \
$(GEN_CMDS__FLAG) \
-l"ti/display/lib/display.aem4f" \
-l"ti/grlib/lib/ccs/m4f/grlib.a" \
-l"third_party/spiffs/lib/ccs/m4f/spiffs.a" \
-l"ti/drivers/lib/drivers_msp432p401x.aem4f" \
-l"third_party/fatfs/lib/ccs/m4f/fatfs.a" \
-lfreertos_builds_MSP_EXP432P401R_release_ccs.lib \
-l"ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Thermostat.out \

EXE_OUTPUTS__QUOTED += \
"Thermostat.out" \

BIN_OUTPUTS += \
Thermostat.hex \

BIN_OUTPUTS__QUOTED += \
"Thermostat.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "Thermostat.out"

dependents:
	-cd C:/ti/ccs1110/ccs/workspace/freertos_builds_MSP_EXP432P401R_release_ccs/Debug && $(MAKE) all

# Tool invocations
Thermostat.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS) C:/ti/ccs1110/ccs/workspace/freertos_builds_MSP_EXP432P401R_release_ccs/Debug/freertos_builds_MSP_EXP432P401R_release_ccs.lib
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O4 --opt_for_speed=1 --advice:power="all" --advice:power_severity=warning --define=BOARD_DISPLAY_USE_UART_ANSI=1 --define=BOARD_DISPLAY_USE_LCD=1 --define=DeviceFamily_MSP432P401x --define=__MSP432P401R__ -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"Thermostat.map" -i"C:/ti/simplelink_msp432p4_sdk_3_40_01_02/source" -i"C:/ti/ccs1110/ccs/workspace/freertos_builds_MSP_EXP432P401R_release_ccs/Debug" -i"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/lib" --define=DeviceFamily_MSP432P401x --define=__MSP432P401R__ --diag_suppress=10063 --diag_wrap=off --display_error_number --warn_sections --xml_link_info="Thermostat_linkInfo.xml" --rom_model -o "Thermostat.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

Thermostat.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: Arm Hex Utility'
	"C:/ti/ccs1110/ccs/tools/compiler/ti-cgt-arm_20.2.5.LTS/bin/armhex" --diag_wrap=off -o "Thermostat.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "DS3231.obj" "DS3231_I2C.obj" "DisplayConsole.obj" "Keypad.obj" "LCDdisplayClient.obj" "LCDdisplayI2C.obj" "LCDdisplayProxy.obj" "MSP_EXP432P401R.obj" "RTC.obj" "TempController.obj" "TempData.obj" "TempSensor.obj" "TempSensorProxy.obj" "mainThread.obj" "main_freertos.obj" "utils.obj" 
	-$(RM) "DS3231.d" "DS3231_I2C.d" "DisplayConsole.d" "Keypad.d" "LCDdisplayClient.d" "LCDdisplayI2C.d" "LCDdisplayProxy.d" "MSP_EXP432P401R.d" "RTC.d" "TempController.d" "TempData.d" "TempSensor.d" "TempSensorProxy.d" "mainThread.d" "main_freertos.d" "utils.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
C:/ti/ccs1110/ccs/workspace/freertos_builds_MSP_EXP432P401R_release_ccs/Debug/freertos_builds_MSP_EXP432P401R_release_ccs.lib:

-include ../makefile.targets

