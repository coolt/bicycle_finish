
#include <inc/hw_memmap.h>
#include <inc/hw_rfc_dbell.h>
#include <inc/hw_rfc_pwr.h>
#include <inc/hw_fcfg1.h>
#include <radio_files/rfc_api/common_cmd.h>
#include <radio_files/rfc_api/ble_cmd.h>
#include <radio_files/rfc_api/mailbox.h>
#include <radio_files/patches/ble/apply_patch.h>
#include <radio_files/overrides/ble_overrides.h>


#include <config.h>
#include <driverLib/prcm.h>
#include <radio.h>
#include <system.h>

/*
volatile bool rfBootDone          = 0;
volatile bool rfSetupDone         = 0;
volatile bool rfAdvertisingDone   = 0;
*/


// Advertisment data. Must be global for other files to access it.
#pragma data_alignment=4
char advData[ADVLEN] = {0};


#pragma data_alignment=8
static uint64_t devAddress;


#pragma data_alignment=4
rfCoreHal_bleAdvPar_t cmdAdvParam = {
  .advLen                     = ADVLEN,
  .pAdvData                   = (uint8_t*)advData,
  .pDeviceAddress             = (uint16_t*)&devAddress,
  .endTrigger.triggerType     = TRIG_NEVER,
};

#pragma data_alignment=4
rfCoreHal_bleAdvOutput_t advOutput = {0};

#pragma data_alignment=4
rfCoreHal_CMD_BLE_ADV_NC_t cmdAdv2 =  {
  .commandNo                  = CMD_BLE_ADV_NC,
  .pNextOp                    = NULL,
  .condition.rule             = COND_ALWAYS,
  .startTrigger.triggerType   = TRIG_NOW,
  .channel                    = 39,
  .pParams                    = (uint8_t*)&cmdAdvParam,
  .pOutput                    = (uint8_t*)&advOutput,
};

#pragma data_alignment=4
rfCoreHal_CMD_BLE_ADV_NC_t cmdAdv1 =  {
  .commandNo                  = CMD_BLE_ADV_NC,
  .pNextOp                    = NULL,
  .condition.rule             = COND_ALWAYS,
  .startTrigger.triggerType   = TRIG_NOW,
  .channel                    = 38,
  .pParams                    = (uint8_t*)&cmdAdvParam,
  .pOutput                    = (uint8_t*)&advOutput,
};


#pragma data_alignment=4
rfCoreHal_CMD_BLE_ADV_NC_t cmdAdv0 =  {
  .commandNo                  = CMD_BLE_ADV_NC,
  .pNextOp                    = NULL,
  .condition.rule             = COND_ALWAYS,
  .startTrigger.triggerType   = TRIG_NOW,
  .channel                    = 37,
  .pParams                    = (uint8_t*)&cmdAdvParam,
  .pOutput                    = (uint8_t*)&advOutput,
};

#pragma data_alignment=4
rfCoreHal_CMD_FS_POWERDOWN_t cmdFsPd = {
  .commandNo                = CMD_FS_POWERDOWN,
  .startTrigger.triggerType = TRIG_NOW,
  .condition.rule           = COND_NEVER,
};

//#pragma data_alignment=4
//rfCoreHal_CMD_RADIO_SETUP_t cmdSetup = {
//  .commandNo                = CMD_RADIO_SETUP,
//  .pNextOp                  = (uint8_t*)&cmdAdv0,
//  .startTrigger.triggerType = TRIG_NOW,
//  .condition.rule           = COND_ALWAYS,
//  .pRegOverride             = bleSingleOverrides,
//  .config.frontEndMode      = 0x1, // Differential
//  .config.biasMode          = 0x1, // Internal bias
//  .txPower.GC               = 0x1, // 0dbm
//  .txPower.IB               = 0x2C, // 0dbm
//  .txPower.tempCoeff        = 0x56,
//  .mode                     = 0, //BLE mode
//
//};

#pragma data_alignment=4
rfCoreHal_CMD_RADIO_SETUP_t cmdSetup = {
  .commandNo                = CMD_RADIO_SETUP,
  .pNextOp                  = (uint8_t*)&cmdAdv0,
  .startTrigger.triggerType = TRIG_NOW,
  .condition.rule           = COND_ALWAYS,
  .pRegOverride             = bleDifferentialOverrides,
  .config.frontEndMode      = 0x0, // Differential
  .config.biasMode          = 0x0, // Internal bias
  .txPower.GC               = 0x1, // 0dbm
  .txPower.IB               = 0x21, // 0dbm
  .txPower.tempCoeff        = 0x31,
  .mode                     = 0, //BLE mode
};



void initRadio(void) {
  // Set radio to BLE mode
  HWREG(PRCM_BASE + PRCM_O_RFCMODESEL) = 0x1;

   //Set up MAC address. Currently using TI Provided adress
   devAddress = *((uint64_t*)(FCFG1_BASE+FCFG1_O_MAC_BLE_0));

  //Chain advertisment commands.
  cmdAdv0.pNextOp = (uint8_t *)&cmdAdv1;
  cmdAdv1.pNextOp = (uint8_t *)&cmdAdv2;
  cmdAdv2.pNextOp = (uint8_t *)&cmdFsPd;
}

void runRadio(void) {
  // Enable clock to CPE, CPE RAM and RF Core
  HWREG(RFC_PWR_NONBUF_BASE + RFC_PWR_O_PWMCLKEN) = RFC_PWR_PWMCLKEN_CPE |  RFC_PWR_PWMCLKEN_CPERAM | RFC_PWR_PWMCLKEN_RFC;
}

 //Send command pointer to doorbell
static inline void radioSendCommand(uint32_t cmd) {
  HWREG( RFC_DBELL_BASE + RFC_DBELL_O_CMDR ) = cmd;
}

//Timed wait for radio direct commands to complete
static inline void radioWaitCommandOk(void) {
  while( HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA) == CMDSTA_Pending);
}

//CM0 patching
void radioPatch(void) {
  applyPatch();
}

void radioCmdBusRequest(bool enabled) {
  // Send bus request command as direct command
  uint32_t cmd = CMDR_DIR_CMD_1BYTE(CMD_BUS_REQUEST, enabled);
  radioSendCommand(cmd);
  radioWaitCommandOk();
}


void radioCmdStartRAT(void) {
  uint32_t cmd = CMDR_DIR_CMD(CMD_START_RAT);
  radioSendCommand(cmd);
  radioWaitCommandOk();
}


void radioSetupAndTransmit() {
  radioSendCommand( (uint32_t)&cmdSetup);
}

//Update advertising byte based on IO inputs
void radioUpdateAdvData(int size, char* data) {

  int i;
  for(i = 0; i < size; i++)
  {
	  advData[i] = data[i];
  }
}


void enableRadioInterrupts(void) {

  // Enable interrupt for BOOT_DONE and LAST_CMD_DONE
  uint32_t intVecs = RFC_DBELL_RFCPEIEN_BOOT_DONE_M |
                     RFC_DBELL_RFCPEIEN_COMMAND_DONE_M |
                     RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

  HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) = intVecs;
}



void initRadioInts(void) {

  // Enable interrupt for BOOT_DONE and LAST_CMD_DONE
  uint32_t intVecs = RFC_DBELL_RFCPEIEN_BOOT_DONE_M |
                     RFC_DBELL_RFCPEIEN_COMMAND_DONE_M |
                     RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

  HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) = intVecs;
}



