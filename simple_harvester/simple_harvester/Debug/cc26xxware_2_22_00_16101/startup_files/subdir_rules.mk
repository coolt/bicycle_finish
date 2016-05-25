################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
cc26xxware_2_22_00_16101/startup_files/startup_ccs.obj: ../cc26xxware_2_22_00_16101/startup_files/startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS/include" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/interfaces" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/sensors" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/radio_files" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/radio_files/overrides" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/radio_files/pa_table" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/radio_files/patches" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/radio_files/rfc_api" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/cc26xxware_2_22_00_16101" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/cc26xxware_2_22_00_16101/inc" --include_path="C:/Users/katrin/Documents/GitHub/bicycle_finish/simple_harvester/simple_harvester/cc26xxware_2_22_00_16101/linker_files" -g --define=DEBUG --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="cc26xxware_2_22_00_16101/startup_files/startup_ccs.d" --obj_directory="cc26xxware_2_22_00_16101/startup_files" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


