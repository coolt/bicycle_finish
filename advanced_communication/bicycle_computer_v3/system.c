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

// globale variable
uint32_t g_timestamp1, g_timestamp2;


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

/*	// Ch 2: Speed Meausrement  -> init by calling
	// -----------------------
	AONRTCCompareValueSet(AON_RTC_CH2, WAKE_INTERVAL_MIDDLE_ENERGY);		// Set RTC ch2 initial compare value: erster Intrupt, der ausgelöst wird
	AONRTCIncValueCh2Set(WAKE_INTERVAL_MIDDLE_ENERGY);						// Set RTC ch 2 auto increment: Nach welcher Zeit der zweite Interrupt ausgelöst wird
	AONRTCModeCh2Set(AON_RTC_MODE_CH2_CONTINUOUS);							// Set RTC CH 2 to auto increment mode ??????
	AONRTCChannelEnable(AON_RTC_CH2);										// Enable channel 2
*/

	// Enable RTC
	AONRTCEnable();
}


void initGPIOInterrupts(void){

	 // Config IOID4 for external interrupt on rising edge and wake up
	 IOCPortConfigureSet(BOARD_IOID_KEY_RIGHT, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_FALLING_EDGE | IOC_INT_ENABLE | IOC_IOPULL_UP | IOC_INPUT_ENABLE | IOC_WAKE_ON_LOW);


	// REED_SWITCH = IOID_25, external interrupt on rising edge and wake up
	//IOCPortConfigureSet(REED_SWITCH, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_FALLING_EDGE | IOC_INT_ENABLE | IOC_IOPULL_UP | IOC_INPUT_ENABLE | IOC_WAKE_ON_LOW);
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

	// RFC Int enable: (baek, temporary) See rfc.h
	// RFCCpe0IntEnable(uint32_t ui32Mask);   // Enable CPE0 Interrupt
}


// Payload buffer = ADV DATA in ADV structure
// BLE-Packet = 62 bytes, therefore 37 bytes of data (in payloadbuffer)
void initBLEBuffer(void){

	memset(payload, 0, ADVLEN); 											// Clear payload buffer

	// general
	payload[0] = ADVLEN - 1; 												// length = ADV-Length - 1 (1 Byte)
	payload[1] = 0x03; 														// Type (1 Byte)  =>   0x03 = UUID -> immer 2 Bytes
	payload[2] = 0xDE; 														// UUID (2 Bytes) =>   0xDE00 (UUID im Ines)
	payload[3] = 0x00;

	// speed  in ms															// Typ is float (4 Bytes)
	payload[4] = 0;
	payload[5] = 0;
	payload[6] = 0;
	payload[7] = 0;

	// sensors
	payload[8] = 0;															// Sensor 1: Drucksensor
	payload[9] = 0;
	payload[10] = 0;														// Sensor 2
	payload[11] = 0;
	payload[12] = 0;														// Sensor 3
	payload[13] = 0;

	// check
	payload[14] = 0;														// Laufnummer (Sequenznummer, ob Packet fehlt)
	payload[15] = 0;
	// ev. checksum
}

// **********************************************************************************************

long getEnergyStateFromSPI(void){

	g_current_energy_state = LOW_ENERGY;

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

void sensorsInit(void)
{
	// biometric pressure senosor
	configure_bmp_280(0);

	//Turn off TMP007
    configure_tmp_007(0);

	//Power down Gyro
	// IOCPinTypeGpioOutput(IOID_12); // github 26. Nov 15
	// GPIOPinClear(1 << IOID_12);		// github 26. Nov 15
	IOCPinTypeGpioOutput(BOARD_IOID_MPU_POWER); // dario
	GPIOPinClear(BOARD_MPU_POWER); // dario

	//Power down Mic
	IOCPinTypeGpioOutput(BOARD_IOID_MIC_POWER);
	GPIOPinClear(1 << BOARD_IOID_MIC_POWER);

	//Turn off external flash
	ext_flash_init(); //includes power down instruction

	//Turn off OPT3001
	configure_opt_3001(0);

	configure_bmp_280(0);

	//Power off Serial domain (Powered on in sensor configurations!)
	PRCMPowerDomainOff(PRCM_DOMAIN_SERIAL);
	while((PRCMPowerDomainStatus(PRCM_DOMAIN_SERIAL) != PRCM_DOMAIN_POWER_OFF));

	//Shut down I2C
	// board_i2c_shutdown();
}

void ledInit(void)
{
	IOCPinTypeGpioOutput(BOARD_IOID_LED_1); //LED1
	IOCPinTypeGpioOutput(BOARD_IOID_LED_2); //LED2

	GPIOPinClear(BOARD_LED_1);
	GPIOPinClear(BOARD_LED_2);
}

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
