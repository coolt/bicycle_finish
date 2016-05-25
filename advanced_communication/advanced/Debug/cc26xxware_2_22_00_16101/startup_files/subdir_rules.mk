################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
cc26xxware_2_22_00_16101/startup_files/startup_ccs.obj: ../cc26xxware_2_22_00_16101/startup_files/startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/workspace_v4/meli16/radio_files" --include_path="C:/Users/katrin/workspace_v4/meli16/interfaces" --include_path="C:/Users/katrin/workspace_v4/meli16/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/advanced_communication/advanced" --include_path="C:/Users/katrin/workspace_v4/meli16/cc26xxware_2_22_00_16101" -g --diag_wrap=off --diag_warning=225 --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="cc26xxware_2_22_00_16101/startup_files/startup_ccs.d" --obj_directory="cc26xxware_2_22_00_16101/startup_files" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


