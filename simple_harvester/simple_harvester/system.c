#include <driverLib/aux_wuc.h>
#include <driverLib/cpu.h>
#include <inc/hw_aon_wuc.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_nvic.h>
#include <inc/hw_prcm.h>
#include <inc/hw_types.h>
#include <driverLib/osc.h>
#include <driverLib/vims.h>


////////////////////////////

#include "config.h"
#include "system.h"
#include <driverLib/aux_wuc.h>
#include <driverLib/cpu.h>
#include <inc/hw_aon_wuc.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_nvic.h>
#include <inc/hw_prcm.h>
#include <inc/hw_types.h>
#include <driverLib/osc.h>
#include <driverLib/vims.h>


// Real Time Clock
#include "config.h"
#include "cc26xxware_2_22_00_16101/driverLib/aon_rtc.h"
#include "cc26xxware_2_22_00_16101/driverLib/sys_ctrl.h"


// GPIO
#include "cc26xxware_2_22_00_16101/driverLib/gpio.h" 						// Konstanten GPIO Pins
#include "board.h" 															// Konstanten IO
#include "cc26xxware_2_22_00_16101/driverLib/ioc.h"  						// Grundeinstellungen aktivieren domains
#include "cc26xxware_2_22_00_16101/inc/hw_aon_event.h"

// RFC
#include "radio.h"
#include "string.h" 					// memset()

// Sensors
#include "sensor-common.h"
#include "ext-flash.h"
#include "bmp-280-sensor.h"				// barometric pressure
#include "tmp-007-sensor.h"				// temperature
#include "hdc-1000-sensor.h"			//  Humitiy
#include "opt-3001-sensor.h"
#include "interfaces/board-i2c.h"
// spi
#include "cc26xxware_2_22_00_16101/driverLib/prcm.h"
#include "spi.h"

// globale variable
uint32_t g_timestamp1, g_timestamp2;
bool g_pressure_set;					// pressure sensor state
bool g_temp_active;
bool g_humidity_acitve;
long g_current_energy_state;
char payload[ADVLEN];

void initWUCEvent(){

	AONRTCCombinedEventConfig(AON_RTC_CH0 | AON_RTC_CH2);  					// Set all used channels to Event-Fabric

	 // Set Interrupt to WUC. Wake MCU afther this Interrupts
	 // WUC0 = RTC2
	 // WUC1 = All GPIO
	 // WUC2 = RTC0
	 HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) = AON_EVENT_MCUWUSEL_WU0_EV_RTC_CH0 | AON_EVENT_MCUWUSEL_WU1_EV_PAD | AON_EVENT_MCUWUSEL_WU2_EV_RTC_CH2 ;
}


void initRTCInterrupts(void) {

	// Ch 0: Wake up
	// --------------
	AONRTCCompareValueSet(AON_RTC_CH0, WAKE_INTERVAL_MIDDLE_ENERGY); 		// Inital Wake up value  (= 10 s)
	AONRTCChannelEnable(AON_RTC_CH0);										// Enable channel 0

	// Ch 2: Speed Meausrement  -> init by calling

}


void initGPIOInterrupts(void){

	// Config IOID4 for external interrupt on rising edge and wake up
	IOCPortConfigureSet(BOARD_IOID_KEY_RIGHT, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_FALLING_EDGE | IOC_INT_ENABLE | IOC_IOPULL_UP | IOC_INPUT_ENABLE | IOC_WAKE_ON_LOW);


	// REED_SWITCH = IOID_25, external interrupt on rising edge and wake up
	IOCPortConfigureSet(REED_SWITCH, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_RISING_EDGE | IOC_INT_ENABLE | IOC_IOPULL_DOWN | IOC_INPUT_ENABLE | IOC_WAKE_ON_HIGH);

	// BAT_LOW = IOID_28, external interrupt on rising edge and wake up
	IOCPortConfigureSet(BAT_LOW, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_FALLING_EDGE | IOC_INT_ENABLE | IOC_IOPULL_UP | IOC_INPUT_ENABLE | IOC_WAKE_ON_LOW);

	// Clear GPIO Register
	HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) & GPIO_O_DOUTCLR31_0;

}


void initRFInterrupts(void) { // hiess vorher: initInterrupts

	// RFCore (=CPE) Interrupts
	// CPE1 - Int channels 31:16: Boot done is bit 30
	HWREG(NVIC_EN0) = 1 << (INT_RF_CPE1 - 16);
	// CPE0 - Int channels  15:0: CMD_DONE is bit 1, LAST_CMD_DONE is bit 0
	HWREG(NVIC_EN0) = 1 << (INT_RF_CPE0 - 16);
	// RTC combined event output
	HWREG(NVIC_EN0) = 1 << (INT_AON_RTC - 16);

}

void initSensors(void){

	// pressor 									-> ..-sensor.c
	enable_bmp_280(1);							// set register for sensor
	init_bmp_280();								// set up I2C for bmp, config and clear
	// enable_bmp_280(0);

	configure_tmp_007(0);

	init_hdc_1000();

	ext_flash_init(); 							//includes power down instruction

	// Power down not needed Sensors
	// Gyro
	IOCPinTypeGpioOutput(BOARD_IOID_MPU_POWER);
	GPIOPinClear(BOARD_MPU_POWER);
	// Mic
	IOCPinTypeGpioOutput(BOARD_IOID_MIC_POWER);
	GPIOPinClear(1 << BOARD_IOID_MIC_POWER);
	// OPT3001
	configure_opt_3001(0);

	/*
	//Power off Serial domain (Powered on in sensor configurations!)
	PRCMPowerDomainOff(PRCM_DOMAIN_SERIAL);
	while((PRCMPowerDomainStatus(PRCM_DOMAIN_SERIAL) != PRCM_DOMAIN_POWER_OFF));

	//Shut down I2C
	board_i2c_shutdown();
 */

	g_temp_active = false;
	g_pressure_set = false;
	g_humidity_acitve = false;

}


// Payload buffer = ADV DATA in ADV structure
// BLE-Packet = 62 bytes, therefore 37 bytes of data (in payloadbuffer)
void initBLEBuffer(void){

	memset(payload, 0, ADVLEN); 											// Clear payload buffer (ADVLEN = 24)

	//header:
	payload[0] = ADVLEN - 1; 												// length = ADV-Length - 1 (1 Byte) = 23 Bytes
	payload[1] = 0x03; 														// Type (1 Byte)  =>   0x03 = UUID -> immer 2 Bytes
	payload[2] = 0xDE; 														// UUID (2 Bytes) =>   0xDE00 (UUID im Ines)
	payload[3] = 0xBA;
	payload[4] = 0;															// Laufnummer für 2 Tage Laufzeit (2 Bytes)
	payload[5] = 0;

	// speed																// 4 bytes
	payload[6] = 0;															// seconds: higher byte
	payload[7] = 0;															// seconds: lower byte
	payload[8] = 0;															// miliseconds: higher byte
	payload[9] = 0;															// miliseconds: lower byte

	// sensor pressure
	payload[10] = 0;														// Sensor 1: Druck (4 bytes: 3 bytes used)
	payload[11] = 0;
	payload[12] = 0;
	payload[13] = 0;

	// sensor temperature
	payload[14] = 0;														// Sensor 2: Temperatur (4 bytes)
	payload[15] = 0;
	payload[16] = 0;
	payload[17] = 0;

	// sensor humidity
	payload[18] = 0;														// Sensor 3: Feuchtigkeit (4 bytes)
	payload[19] = 0;
	payload[20] = 0;
	payload[21] = 0;

	// check
	payload[22] = 0;														// Checksumme: Laufnummer + Checksumme = 0 (Überlauf)
	payload[23] = 0;

}


void initSPI(void){

	// power on
	powerEnableAuxForceOn(); 								// WUC domain
	powerEnableXtalInterface(); 							// clk WUC
	powerEnablePeriph();
	powerEnableGPIOClockRunMode();
	while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON)); /* Wait for domains to power on */

}


// **********************************************************************************************

long getEnergyStateFromSPI(void){

	g_current_energy_state = MIDDLE_ENERGY;		// inital state
	uint8_t energy_status_byte = 0;

	uint8_t lts_bat_min_hi = 0;  		// bit 7
	uint8_t lts_bat_min_lo = 0;			// bit 6
	uint8_t sts_bat_max_hi = 0;			// bit 5
	uint8_t sts_bat_max_lo = 0;			// bit 4
	uint8_t sts_apl_min_hi = 0;			// bit 3
	uint8_t sts_apl_min_lo = 0;			// bit 2
	uint8_t sts_bat_min_hi = 0;			// bit 1
	uint8_t sts_bat_min_lo = 0;			// bit 0


	energy_status_byte = readStatusRegisterEM8500();

	// extract bit for energy state
	lts_bat_min_hi = (energy_status_byte & 0x80) >> 7; // V_LTS > bat_min hi dis
	lts_bat_min_lo = (energy_status_byte & 0x40) >> 6; // V_LTS > bat_min hi con
	sts_bat_max_hi = (energy_status_byte & 0x20) >> 5;
	sts_bat_max_lo = (energy_status_byte & 0x10) >> 4;
	sts_apl_min_hi = (energy_status_byte & 0x08) >> 3;
	sts_apl_min_lo = (energy_status_byte & 0x04) >> 2;
	sts_bat_min_hi = (energy_status_byte & 0x02) >> 1;
	sts_bat_min_lo = (energy_status_byte & 0x01);

	if (sts_apl_min_hi == 1 | sts_apl_min_lo ){
		g_current_energy_state = MIDDLE_ENERGY;
	}
	else if (lts_bat_min_hi == 1 | lts_bat_min_lo == 1   ){

		g_current_energy_state = HIGH_ENERGY;
	}
	return g_current_energy_state;
}


long getEnergyStateFromGPIO(void){

	g_current_energy_state = MIDDLE_ENERGY;

	return g_current_energy_state;

}


uint32_t getTime(void){

	//float time_ms = 0;
	//float time_float = 0;

	// calculate time of wheel cycle
	uint32_t timeDiff = g_timestamp2 - g_timestamp1;

	// convert from register-format to ms
	//timeDiff = timeDiff * 1000; 				// baek: in 2 Schritten, wegen overflow bei float
	//time_float = timeDiff;
	//time_ms = time_float / 65535.0;

	// Reset Timevalues
	g_timestamp1 = 0;
	g_timestamp2 = 0;

	//time_ms = 0x10203040;

	return (uint32_t)(timeDiff);

}

// **********************************************************************************************





// **********************************************************************************************

void powerEnableAuxForceOn(void) {
  HWREGBITW(AON_WUC_BASE + AON_WUC_O_AUXCTL,AON_WUC_AUXCTL_AUX_FORCE_ON_BITN)=1;
}

void powerDisableAuxForceOn(void) {
  HWREGBITW(AON_WUC_BASE + AON_WUC_O_AUXCTL,AON_WUC_AUXCTL_AUX_FORCE_ON_BITN)=0;
}

void powerEnableCache(void) {
  VIMSModeSafeSet(VIMS_BASE, VIMS_MODE_ENABLED, true);
}
void powerDisableCache(void) {
  VIMSModeSafeSet(VIMS_BASE, VIMS_MODE_OFF, true);
}
void powerEnableCacheRetention(void) {
  // Enable cache SRAM retention
  HWREG(PRCM_BASE + PRCM_O_RAMRETEN) |= PRCM_RAMRETEN_VIMS_M;
}
void powerDisableCacheRetention(void) {
  // Disable cache SRAM retention
  HWREG(PRCM_BASE + PRCM_O_RAMRETEN) &= ~PRCM_RAMRETEN_VIMS_M;
}

void powerDisableAuxRamRet(void) {
  // Turn off AUX cache RAM retention
  HWREGBITW(AON_WUC_BASE + AON_WUC_O_AUXCFG,AON_WUC_AUXCFG_RAM_RET_EN_BITN) = 0;
}

void powerEnableRFC(void) {
  // Enable RF Core power domain
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL0RFC , PRCM_PDCTL0RFC_ON_BITN) = 1;
}
void powerDisableRFC(void) {
  // Enable RF Core power domain
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL0RFC , PRCM_PDCTL0RFC_ON_BITN) = 0;
}

void powerEnablePeriph(void) {
  // Enable PERIPH for GPIO access
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL0PERIPH , PRCM_PDCTL0PERIPH_ON_BITN) = 1;
  // Load clock settings
  HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;
}

void powerDisablePeriph(void) {
  // Disable PERIPH for GPIO access
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL0PERIPH , PRCM_PDCTL0PERIPH_ON_BITN) = 0;
  // Load clock settings
  HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;
}

void powerDisableCPU(void) {
  // Turn off CPU domain in CPU Deep Sleep
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL1CPU, PRCM_PDCTL1CPU_ON_BITN) = 0;
}

void powerEnableFlashInIdle(void) {
  // Enable flash when CPU is off
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL1VIMS, PRCM_PDCTL1VIMS_ON_BITN) = 1;
}
void powerDisableFlashInIdle(void) {
  // Disable flash when CPU is off
  HWREGBITW(PRCM_BASE + PRCM_O_PDCTL1VIMS, PRCM_PDCTL1VIMS_ON_BITN) = 0;
}

// Make AUX request powerdown mode and disable AUX RAM retention
// Will not take effect until the bit AON_WUC:AUXCTL:FORCE_ON is cleared
void powerEnableAUXPdReq(void) {
  // Make AUX request power down. Will only take effect when force on bit is not set
  HWREGBITW(AUX_WUC_BASE + AUX_WUC_O_PWRDWNREQ, AUX_WUC_PWRDWNREQ_REQ_BITN) = 1;
  HWREGBITW(AUX_WUC_BASE + AUX_WUC_O_MCUBUSCTL, AUX_WUC_MCUBUSCTL_DISCONNECT_REQ_BITN) = 1;
}

//Divide inf clock in Deep Sleep (for IOC, WUC, WDT etc). Will add latency to interrupt handling!
void powerDivideInfClkDS(uint32_t div) {
  HWREG(PRCM_BASE + PRCM_O_INFRCLKDIVDS) = div;
  // Load clock settings
  HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;
}

// Enable MCU to request to be powered down when CPU is turned off
void powerEnableMcuPdReq(void) {
  HWREGBITW(PRCM_BASE + PRCM_O_VDCTL,PRCM_VDCTL_ULDO_BITN) = 1;
}
// Disable MCU to request to be powered down when CPU is turned off
void powerDisableMcuPdReq(void) {
  HWREGBITW(PRCM_BASE + PRCM_O_VDCTL,PRCM_VDCTL_ULDO_BITN) = 0;
}


void powerEnableGPIOClockRunMode() {
  // Enable clock for GPIO in CPU run mode
  HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 1;
  // Load clock settings
  HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;
}


void powerConfigureRecharge(void) {

//  HWREG(AON_WUC_BASE + AON_WUC_O_RECHARGECFG) = 0x8095C79D;
  AONWUCRechargeCtrlConfigSet(true, 34, 13107, 15000);
}

void powerEnableXtal(void) {
  //Enable HF XTAL for HF clock
  HWREGBITW(AUX_DDI0_OSC_BASE + DDI_0_OSC_O_CTL0, DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_BITN) = 1;
}

void powerDisableXtal(void) {
  //Disable HF XTAL for HF clock
  HWREGBITW(AUX_DDI0_OSC_BASE + DDI_0_OSC_O_CTL0, DDI_0_OSC_CTL0_SCLK_HF_SRC_SEL_BITN) = 0;
}

void powerEnableXtalInterface(void) {
  //Enable clock for OSC interface
  HWREG(AUX_WUC_BASE + AUX_WUC_O_MODCLKEN0) |= AUX_WUC_OSCCTRL_CLOCK;
}

void powerEnableSPIdomain(void){

	// power on GPIO (CS und andere Pins sind GPIO PIns)
	powerEnablePeriph();
	powerEnableGPIOClockRunMode();
		while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON));

	PRCMPowerDomainOn(PRCM_DOMAIN_SERIAL); 		// power on in MCU power domain  -> prcm.cS
	uint32_t debug = HWREG(PRCM_BASE + PRCM_O_SSICLKGR) & PRCM_SSICLKGR_CLK_EN_SSI0; // enable clock for SSI
	HWREG(PRCM_BASE + PRCM_O_SSICLKGR) = 1;
}



void powerDisableSPIdomain(void){
	PRCMDomainDisable(PRCM_DOMAIN_SERIAL);

}

// **********************************************

void waitUntilRFCReady(void) {
  // Wait until RF Core is turned on
  while(HWREGBITW(PRCM_BASE + PRCM_O_PDSTAT0RFC , PRCM_PDSTAT0RFC_ON_BITN) != 1)
  {}
}
void waitUntilPeriphReady(void) {
  // Wait until periph is turned on
  while(HWREGBITW(PRCM_BASE + PRCM_O_PDSTAT0PERIPH, PRCM_PDSTAT0PERIPH_ON_BITN) != 1)
  {}
}
void waitUntilAUXReady(void) {
  // Wait until AUX is powered on and connected to system bus
  while(HWREGBITW(AON_WUC_BASE + AON_WUC_O_PWRSTAT, AON_WUC_PWRSTAT_AUX_PD_ON_BITN) != 1)
  {}
}



//////////////////////////////////////////////////////

// Enable interrupt on CPU
void initInterrupts(void) {

  // CPE1 - Int channels 31:16: Boot done is bit 30
  HWREG(NVIC_EN0) = 1 << (INT_RF_CPE1 - 16);
  // CPE0 - Int channels  15:0: CMD_DONE is bit 1, LAST_CMD_DONE is bit 0
  HWREG(NVIC_EN0) = 1 << (INT_RF_CPE0 - 16);
  // RTC combined event output
  HWREG(NVIC_EN0) = 1 << (INT_AON_RTC - 16);

  // Global interrupt enable
  CPUcpsie();
}


