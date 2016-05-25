
#include <config.h>

#include <inc/hw_aon_event.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <inc/hw_nvic.h>

#include <driverLib/aon_rtc.h>
#include <driverLib/sys_ctrl.h>

long g_current_wake_up_time;
long g_current_energy_state;


// set Wake up time according to energy state
void updateRTCWakeUpTime(long energy_state){

	switch(energy_state){
		case LOW_ENERGY:
			g_current_wake_up_time = WAKE_INTERVAL_LOW_ENERGY;
			break;
		case MIDDLE_ENERGY:
			g_current_wake_up_time = WAKE_INTERVAL_MIDDLE_ENERGY;
			break;
		case HIGH_ENERGY:
			g_current_wake_up_time = WAKE_INTERVAL_HIGH_ENERGY;
			break;
		default:
			g_current_wake_up_time = WAKE_INTERVAL_MIDDLE_ENERGY;
			break;
	}
}


// reduced function AONRTCCurrentCompareValueGet() from aon_rtc.c
// retun value is only Subsec. Sec is ignored
uint32_t AONRTCCurrentSubSecValueGet( void )
{
    uint32_t   ui32CurrentSec    ;
    uint32_t   ui32CurrentSubSec ;
    uint32_t   ui32SecondSecRead ;

    //
    // Reading SEC both before and after SUBSEC in order to detect if SEC incremented while reading SUBSEC
    // If SEC incremented, we can’t be sure which SEC the SUBSEC belongs to, so repeating the sequence then.
    //
    do {
        ui32CurrentSec    = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
        ui32CurrentSubSec = HWREG( AON_RTC_BASE + AON_RTC_O_SUBSEC );
        ui32SecondSecRead = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
    } while ( ui32CurrentSec != ui32SecondSecRead );

    return ui32CurrentSubSec >> 16 ;
    // return (( ui32CurrentSec << 16 ) | ( ui32CurrentSubSec >> 16 ));
}
