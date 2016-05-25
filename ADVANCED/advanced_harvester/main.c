/**
  @file  main.c
  @brief main entry of simpleBroadcaster, a bare-bones, speed optimized
         program transmitting BLE advertisment packages every N ms based on input from 5
         GPIOs

  @usage
        - Make a copy of ccfg.c from your CC26XXWARE version and
          configure it to use internal LF RCOSC
        - Configure WAKE_INTERVAL
        - Configure recharge period to 400ms if WAKE_INTERVAL is larger than 400ms(ish)
        - Configure IO's and set up advertisment payload
        - Configure output power to desired value in CMD_RADIO_SETUP (see pa_table_cc26xx.c)

  <!--
  Copyright 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED ``AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
*/

/*
 * Improved simple broadcast sample (from Ti) edited and optimized.
 * Sensors off --> 3uA sleep current with balanced recharges every ~600ms and peaks arround 10mA
 * (12.01.2016, brts)
 */


#include <driverLib/ioc.h>
#include <driverLib/sys_ctrl.h>
#include <config.h>
#include "string.h"
#include <stdio.h>

// sensors
#include "sensor-common.h"
#include "ext-flash.h"
#include "bmp-280-sensor.h"
#include "tmp-007-sensor.h"
#include "hdc-1000-sensor.h"
#include "opt-3001-sensor.h"

#include "interfaces/board-i2c.h"



#include "mbedtls/aes.h"

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



// SPI
//#include "spi.h"


extern volatile bool rfBootDone;
extern volatile bool rfSetupDone;
extern volatile bool rfAdvertisingDone;

#define TMP007_REG_ADDR_STATUS          0x04
#define TMP_007_SENSOR_TYPE_AMBIENT   2
#define REGISTER_LENGTH                 2
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define SWAP(v) ((LO_UINT16(v) << 8) | HI_UINT16(v))
#define CONV_RDY_BIT                    0x4000


uint32_t g_timestamp1, g_timestamp2;
char payload[ADVLEN];					// shared data buffer
//volatile bool rfBootDone;				// flags RF-Commands
//volatile bool rfSetupDone;
//volatile bool rfAdvertisingDone;
bool g_button_pressed;
bool g_pressure_set;					// pressure sensor state
bool g_temp_active;
bool g_humidity_active;
uint16_t g_pressure;
uint8_t spiBuffer[SPI_BUFFER_LENGTH];

static uint16_t sequenceNumber = 0x0;

// interrupts -----------------------------------------------------------
void GPIOIntHandler(void){
  uint32_t event_flags;
  powerEnablePeriph();
  powerEnableGPIOClockRunMode();

  /* Wait for domains to power on */
  while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON));

  /*Disable interrupts while clearing flags*/
  IntDisable(INT_EDGE_DETECT);
  /* Read interrupt flags */
  event_flags = (HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) & GPIO_PIN_MASK);
  if(event_flags){//Is an event flag set? (should always be set)
    /* Clear the interrupt flags*/
    HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) = event_flags;
    /*Wait until the flag is cleared, no new flag possible because interrupt disabled*/
    while((HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) & GPIO_PIN_MASK));
  }
  /*Enable after flags cleared*/
  IntEnable(INT_EDGE_DETECT);

  powerDisablePeriph();
  // Disable clock for GPIO in CPU run mode
  HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0;
  // Load clock settings
  HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;

  //To avoid second interupt with register = 0 (its not fast enough!!)
  __asm(" nop");
  __asm(" nop");
  __asm(" nop");
  __asm(" nop");
  __asm(" nop");
  __asm(" nop");
}

void sensorsInit(void)
{
	uint16_t success = 0;
    uint16_t val;

	//Turn off TMP007
    configure_tmp_007(0);

	//Power down Gyro
	IOCPinTypeGpioOutput(BOARD_IOID_MPU_POWER);
	GPIOPinClear(BOARD_MPU_POWER);

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
	board_i2c_shutdown();
}

void ledInit(void)
{
	IOCPinTypeGpioOutput(BOARD_IOID_LED_1); //LED1
	IOCPinTypeGpioOutput(BOARD_IOID_LED_2); //LED2

	GPIOPinClear(BOARD_LED_1);
	GPIOPinClear(BOARD_LED_2);
}


int main(void) {

  uint8_t payload[ADVLEN];

  //Disable JTAG to allow for Standby
  AONWUCJtagPowerOff();

  //Force AUX on
  powerEnableAuxForceOn();
  powerEnableRFC();

  powerEnableXtalInterface();
  //powerConfigureRecharge(); --> Optimized version later in this code (brts)
  
  // Divide INF clk to save Idle mode power (increases interrupt latency)
  powerDivideInfClkDS(PRCM_INFRCLKDIVDS_RATIO_DIV32);

  //initRTC();

  powerEnablePeriph();
  powerEnableGPIOClockRunMode();

  /* Wait for domains to power on */
  while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON));

  sensorsInit();
  ledInit();

  //Config IOID4 for external interrupt on rising edge and wake up
  IOCPortConfigureSet(BOARD_IOID_KEY_RIGHT, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_FALLING_EDGE | IOC_INT_ENABLE | IOC_IOPULL_UP | IOC_INPUT_ENABLE | IOC_WAKE_ON_LOW);
  //Set device to wake MCU from standby on PIN 4 (BUTTON1)
  HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) = AON_EVENT_MCUWUSEL_WU0_EV_PAD;  //Does not work with AON_EVENT_MCUWUSEL_WU0_EV_PAD4 --> WHY??

  IntEnable(INT_EDGE_DETECT);

  powerDisablePeriph();
  //Disable clock for GPIO in CPU run mode
  HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0;
  // Load clock settings
  HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;

  initInterrupts();
  initRadio();

  // Turn off FLASH in idle mode
  powerDisableFlashInIdle();

  // Cache retention must be enabled in Idle if flash domain is turned off (to avoid cache corruption)
  powerEnableCacheRetention();

  //AUX - request to power down (takes no effect since force on is set)
  powerEnableAUXPdReq();
  powerDisableAuxRamRet();

  //Clear payload buffer
    memset(payload, 0, ADVLEN);

  while(1) {

    rfBootDone  = 0;
    rfSetupDone = 0;
    rfAdvertisingDone = 0;

    //Wait until RF Core PD is ready before accessing radio
    waitUntilRFCReady();
    initRadioInts();
    runRadio();

    //Wait until AUX is ready before configuring oscillators
    waitUntilAUXReady();

    //Enable 24MHz XTAL
    OSCHF_TurnOnXosc();

    //IDLE until BOOT_DONE interrupt from RFCore is triggered
    while( ! rfBootDone) {
      powerDisableCPU();
      PRCMDeepSleep();
    }

    //This code runs after BOOT_DONE interrupt has woken up the CPU again
    //Request radio to keep on system bus
    radioCmdBusRequest(true);

    //Patch CM0 - no RFE patch needed for TX only
    radioPatch();

    //Start radio timer
    radioCmdStartRAT();

    //Enable Flash access while doing radio setup
    powerEnableFlashInIdle();

    //Switch to XTAL
    while( !OSCHF_AttemptToSwitchToXosc())
    {}
  

    powerEnablePeriph();
    powerEnableGPIOClockRunMode();

     /* Wait for domains to power on */
     while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON));

     sensorsInit();
     ledInit();

     //Start Temp measurement
    enable_tmp_007(1);
    //start hum measurement
    configure_hdc_1000();
    start_hdc_1000();
    //Wait for, read and calc temperature

    int temperature ;
    do{
    	temperature = value_tmp_007(TMP_007_SENSOR_TYPE_AMBIENT);
    }while(temperature==0x80000000);
    enable_tmp_007(0);
    //char char_temp[5] = temperature;


    //Wait for, read and calc humidity
    while(!read_data_hdc_1000())
    	;
    int humidity = value_hdc_1000(HDC_1000_SENSOR_TYPE_HUMIDITY);
    char char_hum[5];
    sprintf(char_hum, "%3d",humidity/10);


//END read sensor values
/*****************************************************************************************/

    powerDisablePeriph();
	// Disable clock for GPIO in CPU run mode
	HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0;
	// Load clock settings
	HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;

/*****************************************************************************************/

	uint32_t timeFromRegister = 0x11111111;
	//timeFromRegister = getTime();

	uint32_t pressure = 0x22222222;



	uint8_t p;
    p = 0;

    payload[p++] = ADVLEN-1;
    payload[p++] = 0x03;
    payload[p++] = 0xDE;
    payload[p++] = 0xBA;
    payload[p++] = (char) (sequenceNumber >> 8);
    payload[p++] = (char) sequenceNumber;

    payload[p++] = (timeFromRegister >> 24) & 0x000000FF;
    payload[p++] = (timeFromRegister >> 16) & 0x000000FF;
    payload[p++] = (timeFromRegister >> 8) & 0x000000FF;
    payload[p++] = timeFromRegister  & 0x000000FF;

    payload[p++] = 0;
    payload[p++] = (pressure >> 16) & 0x000000FF;
    payload[p++] = (pressure >> 8) & 0x000000FF;
    payload[p++] = pressure  & 0x000000FF;

    payload[p++] = 0;
    payload[p++] = 0;
    payload[p++] = (char) (temperature >> 8);
 	payload[p++] = (char) temperature & 0x00FF;
 	//payload[p++] = char_temp[0]; // = 0;
 	//payload[p++] = char_temp[1];
 	//payload[p++] = char_temp[2];


   	payload[p++] = 0;
   	payload[p++] = char_hum[0];
   	payload[p++] = char_hum[1];
   	payload[p++] = char_hum[2];

   	payload[p++] = 0;  // checksum
   	payload[p++] = 0;


   	sequenceNumber++;

    //Start radio setup and linked advertisment
    radioUpdateAdvData(ADVLEN, payload);

    //Start radio setup and linked advertisment
    radioSetupAndTransmit();

//END: Transmit
/*****************************************************************************************/


    //Wait in IDLE for CMD_DONE interrupt after radio setup. ISR will disable radio interrupts
    while( ! rfSetupDone) {
      powerDisableCPU();
      PRCMDeepSleep();
    }

    //Disable flash in IDLE after CMD_RADIO_SETUP is done (radio setup reads FCFG trim values)
    powerDisableFlashInIdle();

    //Wait in IDLE for LAST_CMD_DONE after 3 adv packets
    while( ! rfAdvertisingDone) {
      powerDisableCPU();
      PRCMDeepSleep();
    }

    //Request radio to not force on system bus any more
    radioCmdBusRequest(false);
    
    //
    // Standby procedure
    //
    
    powerDisableXtal();

    // Turn off radio
    powerDisableRFC();
    
    // Switch to RCOSC_HF
    OSCHfSourceSwitch();

    // Allow AUX to turn off again. No longer need oscillator interface
    powerDisableAuxForceOn();

    // Goto Standby. MCU will now request to be powered down on DeepSleep
    powerEnableMcuPdReq();

    // Disable cache and retention
    powerDisableCache();
    powerDisableCacheRetention();

    //Calculate next recharge
    SysCtrlSetRechargeBeforePowerDown(XOSC_IN_HIGH_POWER_MODE);

    // Synchronize transactions to AON domain to ensure AUX has turned off
    SysCtrlAonSync();

    //
    // Enter Standby
    //

    powerDisableCPU();
    PRCMDeepSleep();

    SysCtrlAonUpdate();
    SysCtrlAdjustRechargeAfterPowerDown();
    SysCtrlAonSync();

    //
	// Wakeup from RTC every 100ms, code starts execution from here
	//
   
    powerEnableRFC();
    powerEnableAuxForceOn();

    //Re-enable cache and retention
    powerEnableCache();
    powerEnableCacheRetention();

    //MCU will not request to be powered down on DeepSleep -> System goes only to IDLE
    powerDisableMcuPdReq();
  }
}