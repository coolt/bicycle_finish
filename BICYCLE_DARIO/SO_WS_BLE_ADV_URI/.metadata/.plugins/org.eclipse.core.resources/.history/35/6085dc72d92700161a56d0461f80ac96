
#include <config.h>

#include <inc/hw_aon_event.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <inc/hw_nvic.h>

#include <driverLib/aon_rtc.h>
#include <driverLib/sys_ctrl.h>

void initRTC(void) {
  //Add RTC Ch2 event as input to AON RTC interrupt
  AONRTCCombinedEventConfig(AON_RTC_CH2);
  //Set RTC ch 2 auto increment
  AONRTCIncValueCh2Set(WAKE_INTERVAL_TICKS);
  //Set RTC ch2 initial compare value
  AONRTCCompareValueSet(AON_RTC_CH2, WAKE_INTERVAL_TICKS);
  //Set RTC CH 2 to auto increment mode
  AONRTCModeCh2Set(AON_RTC_MODE_CH2_CONTINUOUS);
  //Enable channel 2
  AONRTCChannelEnable(AON_RTC_CH2);
  //Set device to wake MCU from standby on RTC channel 2
  HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) = AON_EVENT_MCUWUSEL_WU0_EV_RTC_CH2;

  //Enable RTC
  AONRTCEnable();

}

//RTC interrupt handler
void AONRTCIntHandler(void) {

  // Clear RTC event flag
  do{
    AONRTCEventClear(AON_RTC_CH2);
  }
  while( AONRTCEventGet(AON_RTC_CH2));

}

