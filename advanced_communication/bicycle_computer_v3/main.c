/**
 V3:
 * Energy mode und wake up time manully set. (see getdata() )
 *
 */


// Set up
#include <driverLib/ioc.h>				// Grundeinstallung aktive domains
#include <driverLib/sys_ctrl.h>			// Bus, CPU, Refresh
#include <config.h>						// Konstanten Applkation
#include <system.h>						// Funktionen (Power), Init, Waits

// Sensors
#include "sensor-common.h"
#include "ext-flash.h"
#include "bmp-280-sensor.h"				// barometric pressure
#include "tmp-007-sensor.h"
#include "hdc-1000-sensor.h"			// Humitiy
#include "opt-3001-sensor.h"

// GPIO
#include "board.h"						// Konstanten IO
#include <driverLib/gpio.h>				// Konstanten GPIO Pins

// RF-Chip (M0)
#include "radio.h"
#include <driverLib/rfc.h>				// Set up RFC interrupts

// RTC
#include <rtc.h>
#include <driverLib/aon_rtc.h>
#include <inc/hw_aon_event.h>

#include "string.h"


// globale variables: declared in config.h, set in handler (startup_ccs) and radio.c
bool g_measurement_done;				// flag, set when 2 timestamps from reed-switch are stored
uint32_t g_timestamp1, g_timestamp2;
char payload[ADVLEN];					// shared data buffer
volatile bool rfBootDone;				// flags RF-Commands
volatile bool rfSetupDone;
volatile bool rfAdvertisingDone;
bool g_button_pressed;
bool g_pressure_set;					// pressure sensor state
uint16_t g_pressure;

// ------------------------------
// functions
// ------------------------------

void initSensortag(void){

		// power off
		AONWUCJtagPowerOff(); 									//Disable JTAG to allow for Standby

		// power on
		powerEnableAuxForceOn(); 								// WUC domain
		powerEnableXtalInterface(); 							// clk WUC
		powerDivideInfClkDS(PRCM_INFRCLKDIVDS_RATIO_DIV32); 	// Divide INF clk to save Idle mode power (increases interrupt latency)
		powerEnablePeriph();
		powerEnableGPIOClockRunMode();
		while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON)); /* Wait for domains to power on */

		// Configuartions
		sensorsInit();											// Enable needed Sensors
		initRadio();											// Set Communicationmode = BLE, Channels = 3, Advertising modus
		ledInit();
		initBLEBuffer(); 										// Set default structure

		// Configure Interrupts
		initWUCEvent();											// Ch0 = RTC2, Ch1 = all GPIO, Ch2 = RTC0
		initRTCInterrupts();									// CH0: WakeUp, CH2: Speed calculation
		initGPIOInterrupts();									// Define IOPorts for Interrupt, Add GPIO-mask to WU-Event
		initRFInterrupts(); 									// Set RFInterrupts to NVIC


		// Setup for next state
		IntEnable(INT_EDGE_DETECT); // Dario
//		IntDisable(INT_EDGE_DETECT);							// Enable specific interrupt. Int_EDGE_DETECT = Nr. 16  (=> all GPIO-interrupts   ?? )
//		AONRTCEnable();											// PA: Enable RTC

		// power off and set Refresh on
		// -- moved functions from in the middle of interrupt settings
		powerDisablePeriph(); //Disable clock for GPIO in CPU run mode
		HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0;
		HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1; // Load clock settings

		// -- old functions
		powerDisableFlashInIdle();  							// Turn off FLASH in idle mode == stand by mode
		powerEnableCacheRetention(); 							// Cache retention must be enabled in Idle if flash domain is turned off (to avoid cache corruption)
		powerEnableAUXPdReq(); 									//AUX - request to power down (takes no effect since force on is set)
		powerDisableAuxRamRet();
		powerDisableMcuPdReq();									//MCU will not request to be powered down on DeepSleep -> System goes only to IDLE
}


void getData(void){

	// Wakeup from RTC according to energy-state
	// ---------------------------------------------

	// start system
	// powerEnableRFC();
	powerEnableAuxForceOn(); // ??????????????????????' not done in RTC interrupt ??
	powerEnableCache(); // ?????  Wann notwendig ?? immer

	// read STS, LTS to know Energy state
	// ----------------------------------
	g_current_energy_state = getEnergyStateFromSPI();
	updateRTCWakeUpTime(g_current_energy_state);

	// read reed switch twice
	// ----------------------
	//  wait until interrupt set timestamps
/*	if(g_measurement_done){

		// calculate time-difference
		uint32_t g_time_ms = getTime();
		g_time_ms = 0x11223344;
	}
*/
}


void setData(void){

	// Wait for interrupts for data

	// after 2 RTC_CH2 interrupts, time measuring is done
	if(g_measurement_done == true){

		uint32_t timeFromRegister = getTime();

		// extract bytes
		uint8_t higherSeconds    = (timeFromRegister >> 24) & 0x000000FF;
		uint8_t lowerSeconds     = (timeFromRegister >> 16) & 0x000000FF;
		uint8_t higherSubSeconds = (timeFromRegister >> 8) & 0x000000FF;
		uint8_t lowerSubSeconds  = timeFromRegister  & 0x000000FF;


		// set current time to BLE-buffer
		payload[4] =  (char) higherSeconds;
		payload[5] =  (char) lowerSeconds;
		payload[6] =  (char) higherSubSeconds;
		payload[7] =  (char) lowerSubSeconds;

		g_measurement_done = false;

	}

	else if(g_pressure_set == true){

		// payload[8] =  g_pressure & 0x00FF;
		// payload[9] =  (g_pressure >> 8) & 0x00FF;
		payload[8] =  7;
		payload[9] =  7;

		g_pressure_set == false;
	}

	else if(g_button_pressed == true){

		// check bytes
		payload[14] =  0xEE;
		payload[15] =  0xFF;

		g_button_pressed = false;
	}
	else {
		payload[4]  =  (char) 0x0;
		payload[5]  =  (char) 0x0;
		payload[6]  =  (char) 0x0;
		payload[7]  =  (char) 0x0;
		payload[8]  =  (char) 0x0;
		payload[9]  =  (char) 0x0;
		payload[10] =  (char) 0x0;
		payload[11] =  (char) 0x0;
		payload[12] =  (char) 0x0;
		payload[13] =  (char) 0x0;
		payload[14] =  (char) 0x0;
		payload[15] =  (char) 0x0;
	}

	//Start radio setup and linked advertisment
	radioUpdateAdvData(16, payload); 			//Update advertising byte based on IO inputs
}


void sendData(){

	// Flags for RF-Doorbell-communication between CPU M3 and RFC M0
    rfBootDone  = 0;
    rfSetupDone = 0;
    rfAdvertisingDone = 0;

	powerEnableRFC(); 							// Set power bit
	waitUntilRFCReady();
	enableRadioInterrupts();  					// Set enable bit for CPE communication interrupts
	runRadio();									// Power CPU (M3), RAM and CPE (M0)

	waitUntilAUXReady(); 						// AUX is needed to configure higher oscillator
	OSCHF_TurnOnXosc();  						// Enable 24 MHz XTAL (higher clk for sending)
	int debug = rfBootDone; // = 0
	while( ! rfBootDone) { 						// rfBootDone set by CPE interrupt 												!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! hanging here BootDone = 0
		powerDisableCPU();
		PRCMDeepSleep();	//note:				// if commented out: Request radio to keep on system bus
	}
	radioCmdBusRequest(true); 					// Request radio to keep on system bus
	radioPatch(); 								// Patch CM0 - no RFE patch needed for TX only
	radioCmdStartRAT(); 						// Start radio timer
	powerEnableFlashInIdle(); 					// Enable Flash access while doing radio setup
	while( !OSCHF_AttemptToSwitchToXosc()) 		// Switch to XTAL, higher clock
	{}

	// Sending data
	radioSetupAndTransmit();
	while( ! rfSetupDone) {
		powerDisableCPU();
		PRCMDeepSleep();
	}
	powerDisableFlashInIdle(); 					// Disable flashafter CMD_RADIO_SETUP is done (radio setup reads FCFG trim values)
	while( ! rfAdvertisingDone) {				// AdvertisingDone = true, when 3 packets are send
	  powerDisableCPU();
	  PRCMDeepSleep();
	}
	radioCmdBusRequest(false);					// Request radio to not force on system bus any more

	RFCAckIntClear(); 	// add baek.			// clear RFC Interrupts
	AONRTCEnable();		// add baek. 			// defined interrupt state for next state
}


void sleep(){

	// Standby procedure
	powerDisableXtal();
	powerDisableRFC();
	OSCHfSourceSwitch(); 						// lower clk
	powerDisableAuxForceOn(); 					// Higher oscillator interface no more needed
	powerEnableMcuPdReq(); 						// Goto Standby. MCU will now request to be powered down on DeepSleep
	powerDisableCache();
	powerDisableCacheRetention();

	//Calculate next recharge (Refreshtime): must be BEFORE POWER DOWN
	SysCtrlSetRechargeBeforePowerDown(XOSC_IN_HIGH_POWER_MODE);
	SysCtrlAonSync(); 							// Synchronize transactions to AON domain to ensure AUX has turned off

	// Enter Standby
	powerDisableCPU();
	PRCMDeepSleep();
	SysCtrlAonUpdate();
	SysCtrlAdjustRechargeAfterPowerDown();   	// AFTER POWER DOWN: Set refresh cycle
	SysCtrlAonSync();
}


//===============================================================================


int main(void) {

  initSensortag();
  CPUcpsie();												// All extern interrupts enable (globaly)
  g_timestamp1 = 0; 										// bei init löst sich Reed Int erstesmal von selbst aus
  // interrupt driven application
  while(1) {

	// wait for interrupts
	getData();
	setData();
	sendData();
    sleep();
  }
}



