/**
 V4:
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
#include "tmp-007-sensor.h"				// temperature
#include "hdc-1000-sensor.h"			//  Humitiy
#include "opt-3001-sensor.h"

#define SENSOR_HUMIDITY_I2C_ADDRESS     0x43		// -> hdc-1000-sensor.c
#define SENSOR_TEMPERATURE_I2C_ADDRESS  0x44		// temp-007-sensor.c
#define SENSOR_PRESSURE_I2C_ADDRESS     0x77		// bmp-280-sensor.c

#define TMP007_REG_ADDR_STATUS          0x04
#define TMP_007_SENSOR_TYPE_AMBIENT   	2
#define REGISTER_LENGTH                 2
#define HI_UINT16(a) 					(((a) >> 8) & 0xFF)
#define LO_UINT16(a) 					((a) & 0xFF)
#define SWAP(v) 						((LO_UINT16(v) << 8) | HI_UINT16(v))
#define CONV_RDY_BIT                    0x4000

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

// SPI
#include "spi.h"

// globale variables: declared in config.h, set in handler (startup_ccs) and radio.c
bool g_measurement_done;				// flag, set when 2 timestamps from reed-switch are stored
uint32_t g_timestamp1, g_timestamp2, g_time_diff;
char payload[ADVLEN];					// shared data buffer
volatile bool rfBootDone;				// flags RF-Commands
volatile bool rfSetupDone;
volatile bool rfAdvertisingDone;
bool g_button_pressed;
bool g_pressure_set;					// pressure sensor state
bool g_temp_active;
bool g_humidity_active;
uint16_t g_pressure;  					// ?????? variable for value ?
uint8_t spiBuffer[SPI_BUFFER_LENGTH];

static uint16_t sequenceNumber = 0x0;

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
		initRadio();											// Set Communicationmode = BLE, Channels = 3, Advertising modus
		initBLEBuffer(); 										// Set default structure
		initSensors(); 											// Define IOC and enable sensor

		// Configure Interrupts
		initWUCEvent();											// Ch0 = RTC2, Ch1 = all GPIO, Ch2 = RTC0
		initRTCInterrupts();									// CH0: WakeUp, CH2: Speed calculation
		initGPIOInterrupts();									// Define IOPorts for Interrupt, Add GPIO-mask to WU-Event
		initRFInterrupts(); 									// Set RFInterrupts to NVIC


		// Setup for next state
		//IntEnable(INT_EDGE_DETECT); // Dario
//		IntDisable(INT_EDGE_DETECT);							// Enable specific interrupt. Int_EDGE_DETECT = Nr. 16  (=> all GPIO-interrupts   ?? )
		AONRTCEnable();											// PA: Enable RTC

		// power off and set Refresh on
		powerDisablePeriph(); //Disable clock for GPIO in CPU run mode
		HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0;
		HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1; // Load clock settings

		// -- old functions
		powerDisableFlashInIdle();  							// Turn off FLASH in idle mode == stand by mode
		powerEnableCacheRetention(); 							// Cache retention must be enabled in Idle if flash domain is turned off (to avoid cache corruption)
		powerEnableAUXPdReq(); 									//AUX - request to power down (takes no effect since force on is set)
		powerDisableAuxRamRet();
		powerDisableMcuPdReq();									//MCU will not request to be powered down on DeepSleep -> System goes only to IDLE

		// system sleeps(). Waits for RTC Wake up interrupt
}


void getData(void){

	// Wakeup from RTC according to energy-state
	// ---------------------------------------------

	// start system
	powerEnableAuxForceOn();
	powerEnableCache();

	// read Energy state from EM8500
	// ----------------------------------
	//g_current_energy_state = getEnergyStateFromGPIO();
	g_current_energy_state = getEnergyStateFromSPI();   					// dummy value set to MIDDLE
	updateRTCWakeUpTime(g_current_energy_state);

	// clear ble-data-buffer
	memset(payload, 0, ADVLEN); 											// Clear payload buffer (ADVLEN = 24)

	// set header ble-data-buffer
	payload[0] = ADVLEN - 1; 												// length = ADV-Length - 1 (1 Byte) = 23 Bytes
	payload[1] = 0x03; 														// Type (1 Byte)  =>   0x03 = UUID -> immer 2 Bytes
	payload[2] = 0xDE; 														// UUID (2 Bytes) =>   0xDE00 (UUID im Ines)
	payload[3] = 0xBA;
	payload[4] = (char) (sequenceNumber >> 8);															// Laufnummer für 2 Tage Laufzeit (2 Bytes)
	payload[5] = (char) sequenceNumber;


	// read sensors acording to the energy state
	// LOW:  no sensorts
	// MIDDLE: only one sensor, but each time a new one (ringbuffer-system)
	// HIGH: read all sensors
	if(g_current_energy_state == MIDDLE_ENERGY ){

		static int g_ringbuffer = 0;

		if(g_ringbuffer == 0){
			enable_bmp_280(1);
			g_pressure_set = true;
			g_ringbuffer ++;
		}
		else if (g_ringbuffer == 1){
			enable_tmp_007(1);
			g_temp_active = true;
			g_ringbuffer ++;
		}
		else if(g_ringbuffer == 2){
			// start_hdc_1000();
			g_humidity_active = true;
			g_ringbuffer = 0;
		}
	} // end MIDDLE ENERGY

	else if (g_current_energy_state == HIGH_ENERGY ){
		enable_tmp_007(1);
		g_pressure_set = true;
		enable_bmp_280(1);
		g_temp_active = true;
		g_humidity_active = true;
	}

	// update sequence_number
	sequenceNumber++;
}


void setData(void){

	// after 2 RTC_CH2 interrupts, time measuring is done
	if(g_measurement_done){

		// uint32_t timeFromRegister = 0;
		// timeFromRegister = getTime(); 						// done in interrupt

		// extract bytes
		uint8_t higherSeconds    = (g_time_diff >> 24) & 0x000000FF;
		uint8_t lowerSeconds     = (g_time_diff >> 16) & 0x000000FF;
		uint8_t higherSubSeconds = (g_time_diff >> 8) & 0x000000FF;
		uint8_t lowerSubSeconds  = g_time_diff  & 0x000000FF;

		// set current time to BLE-buffer
		payload[6] =  (char) higherSeconds;
		payload[7] =  (char) lowerSeconds;
		payload[8] =  (char) higherSubSeconds;
		payload[9] =  (char) lowerSubSeconds;

		g_measurement_done = false;
		g_time_diff = 0;
	}

	// Athmonspheren-druck bei 450 müM = 96'600 Pa = ca 0.96 bar = 966 hPa
	else if(g_pressure_set){

		uint32_t pressure = 0;  			// only 3 Bytes used
		//uint32_t temp = 0;
		select_bmp_280();     				// activates I2C for bmp-sensor
		enable_bmp_280(1);					// works

		do{
			pressure = value_bmp_280(BMP_280_SENSOR_TYPE_PRESS);  //  read and converts in pascal (96'000 Pa)
			//temp = value_bmp_280(BMP_280_SENSOR_TYPE_TEMP);
		}while(pressure == 0x80000000);

		// extract 3 bytes (from 32 bit data vector)
		uint8_t highestByte = (pressure >> 16) & 0x000000FF;
		uint8_t middleByte  = (pressure >> 8) & 0x000000FF;
		uint8_t lowestByte  = pressure  & 0x000000FF;

		// header data needed, because wrong after read pressure
		payload[0] = ADVLEN - 1; 												// length = ADV-Length - 1 (1 Byte) = 23 Bytes
		payload[1] = 0x03; 														// Type (1 Byte)  =>   0x03 = UUID -> immer 2 Bytes
		payload[2] = 0xDE; 														// UUID (2 Bytes) =>   0xDE00 (UUID im Ines)
		payload[3] = 0xBA;
		payload[4] = (char) (sequenceNumber >> 8);															// Laufnummer für 2 Tage Laufzeit (2 Bytes)
		payload[5] = (char) sequenceNumber;

		// set current pressure to BLE-buffer
		payload[10] =  (char) 0;
		payload[11] =  (char) highestByte;
		payload[12] =  (char) middleByte;
		payload[13] =  (char) lowestByte;

		g_pressure_set = false;
		// enable_bmp_280(0);
		// board_i2c_shutdown();

		}

	else if(g_temp_active){

		uint16_t temperature = 0;

		board_i2c_wakeup();
		enable_tmp_007(1);					// activates I2C
		board_i2c_select(BOARD_I2C_INTERFACE_0, SENSOR_TEMPERATURE_I2C_ADDRESS); 		// activate I2C

		 do{
		    	temperature = value_tmp_007(TMP_007_SENSOR_TYPE_AMBIENT);      // in mC°
		    }while(temperature == 0x80000000 |temperature == 0 );


		// extract bytes
		uint8_t higherByte = (temperature >> 8) & 0x000000FF;
		uint8_t lowerByte  = temperature  & 0x000000FF;

		// set current temp in mC° to BLE-buffer
		payload[14] =  0;
		payload[15] =  0;
		payload[16] =  higherByte;
		payload[17] =  lowerByte;

		g_temp_active = false;
		//enable_tmp_007(0);
		//board_i2c_shutdown();

		}

	else if(g_humidity_active){

			int humidity = 0;

			board_i2c_wakeup();
			board_i2c_select(BOARD_I2C_INTERFACE_0, SENSOR_HUMIDITY_I2C_ADDRESS); 		// activate I2C

			configure_hdc_1000();  // works
		    start_hdc_1000();  // works


		    // wait for reading value
		    while(!read_data_hdc_1000());

		    humidity = value_hdc_1000(HDC_1000_SENSOR_TYPE_HUMIDITY);

			// extract bytes
			uint8_t higherSeconds    = (humidity >> 24) & 0x000000FF;
			uint8_t lowerSeconds     = (humidity >> 16) & 0x000000FF;
			uint8_t higherSubSeconds = (humidity >> 8) & 0x000000FF;
			uint8_t lowerSubSeconds  = humidity  & 0x000000FF;

			// set current time to BLE-buffer
			payload[18] =  (char) higherSeconds;
			payload[19] =  (char) lowerSeconds;
			payload[20] =  (char) higherSubSeconds;
			payload[21] =  (char) lowerSubSeconds;

			g_humidity_active = false;
			//board_i2c_shutdown();
		}


	// to del: only for debugging (set values in check-value buffer)
	else if(g_button_pressed){

		// check bytes
		payload[22] =  0xEE;
		payload[23] =  0xFF;

		g_button_pressed = false;
	}

	// no new data to send: clear buffer
	else {
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
		payload[16] =  (char) 0x0;
		payload[17] =  (char) 0x0;
		payload[18] =  (char) 0x0;
		payload[19] =  (char) 0x0;
		payload[20] =  (char) 0x0;
		payload[21] =  (char) 0x0;
		payload[22] =  (char) 0x0;
		payload[23] =  (char) 0x0;
	}

	//Start radio setup and linked advertisment
	radioUpdateAdvData((ADVLEN-1), payload); 			// Update advertising byte based on IO inputs
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

	// added against crah ...
	HWREG(AON_WUC_BASE + AON_WUC_O_AUXCTL) = AON_WUC_AUXCTL_AUX_FORCE_ON;   // to check !
	waitUntilAUXReady(); 	//CRASH !!!!		// AUX is needed to configure higher oscillator

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

	// enable RTC wake up Interrupt
	AONRTCChannelEnable(AON_RTC_CH0);

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

		/*
		AONWUCJtagPowerOff(); 								//Disable JTAG to allow for Standby
		configureEM8500();
		*/


		// system in wake up
		// wait for interrupts

		//if(!g_measurement_done){
			getData();										// get current energy state
		//}

		//if(g_measurement_done){
			setData();
			sendData();
			sleep();
		//}

	}
}



