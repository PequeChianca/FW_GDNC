################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
App/App.obj: ../App/App.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.6/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -Ooff --fp_mode=relaxed --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.6/include" --include_path="C:/Users/User/CCSworkspace/freertos_demo" --include_path="c:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="c:/ti/TivaWare_C_Series-2.1.2.111" --include_path="c:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="c:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="c:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="c:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power=all -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="App/App.pp" --obj_directory="App" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


