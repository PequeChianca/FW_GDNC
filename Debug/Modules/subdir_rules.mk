################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Modules/CC1100.obj: ../Modules/CC1100.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --include_path="C:/Users/pedro/OneDrive/Documentos/GitHub/WkSpace/FW_GDNC" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/utils" --advice:power=all -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --diag_wrap=off --diag_warning=225 --display_error_number --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="Modules/CC1100.d" --obj_directory="Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Modules/CC2500.obj: ../Modules/CC2500.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.3.LTS/include" --include_path="C:/Users/pedro/OneDrive/Documentos/GitHub/WkSpace/FW_GDNC" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/TivaWare_C_Series-2.1.3.156/utils" --advice:power=all -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --diag_wrap=off --diag_warning=225 --display_error_number --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="Modules/CC2500.d" --obj_directory="Modules" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

