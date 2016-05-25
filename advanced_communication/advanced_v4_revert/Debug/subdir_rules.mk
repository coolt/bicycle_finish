################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
ccfg.obj: ../ccfg.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/dwn_v4_revert" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="ccfg.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/dwn_v4_revert" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="main.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

radio.obj: ../radio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/dwn_v4_revert" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="radio.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

rtc.obj: ../rtc.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/dwn_v4_revert" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="rtc.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

spi.obj: ../spi.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/dwn_v4_revert" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="spi.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

system.obj: ../system.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/dwn_v4_revert" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="system.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


