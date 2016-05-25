/******************************************************************************
*
*
******************************************************************************/

#include <driverLib/aon_rtc.h>
#include <driverLib/cpu.h>

//*****************************************************************************
//
// Handle support for DriverLib in ROM:
// This section will undo prototype renaming made in the header file
//
//*****************************************************************************
#if !defined(DOXYGEN)
    #undef  AONRTCCurrentCompareValueGet
    #define AONRTCCurrentCompareValueGet    NOROM_AONRTCCurrentCompareValueGet
    #undef  AONRTCCurrent64BitValueGet
    #define AONRTCCurrent64BitValueGet      NOROM_AONRTCCurrent64BitValueGet
#endif


//*****************************************************************************
//
// Get the current value of the RTC counter in a format compatible to the compare registers.
//
//*****************************************************************************
uint32_t AONRTCCurrentCompareValueGet( void )
{
    uint32_t   ui32CurrentSec    ;
    uint32_t   ui32CurrentSubSec ;
    uint32_t   ui32SecondSecRead ;

    //
    // Reading SEC both before and after SUBSEC in order to detect if SEC incremented while reading SUBSEC
    // If SEC incremented, we can�t be sure which SEC the SUBSEC belongs to, so repeating the sequence then.
    //
    do {
        ui32CurrentSec    = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
        ui32CurrentSubSec = HWREG( AON_RTC_BASE + AON_RTC_O_SUBSEC );
        ui32SecondSecRead = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
    } while ( ui32CurrentSec != ui32SecondSecRead );

    return (( ui32CurrentSec << 16 ) | ( ui32CurrentSubSec >> 16 ));
}

//*****************************************************************************
//
// Get the current 64-bit value of the RTC counter.
//
//*****************************************************************************
uint64_t AONRTCCurrent64BitValueGet( void )
{
    union {
        uint64_t  returnValue       ;
        uint32_t  secAndSubSec[ 2 ] ;
    } currentRtc                    ;
    uint32_t      ui32SecondSecRead ;

    //
    // Reading SEC both before and after SUBSEC in order to detect if SEC incremented while reading SUBSEC
    // If SEC incremented, we can�t be sure which SEC the SUBSEC belongs to, so repeating the sequence then.
    //
    do {
        currentRtc.secAndSubSec[ 1 ] = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
        currentRtc.secAndSubSec[ 0 ] = HWREG( AON_RTC_BASE + AON_RTC_O_SUBSEC );
        ui32SecondSecRead            = HWREG( AON_RTC_BASE + AON_RTC_O_SEC    );
    } while ( currentRtc.secAndSubSec[ 1 ] != ui32SecondSecRead );

    return ( currentRtc.returnValue );
}
