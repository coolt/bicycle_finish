/******************************************************************************
*
******************************************************************************/


//*****************************************************************************
//
// Check if compiler is CCS
//
//*****************************************************************************
#if !(defined(__TI_COMPILER_VERSION__))
#error "startup_ccs.c: Unsupported compiler!"
#endif


// *********************************************
// Includes for Hanlder
//
// *********************************************

#include "../inc/hw_types.h" 			//default
#include "../../config.h"
#include "../inc/hw_memmap.h"
#include "../driverLib/ioc.h"
#include "../driverLib/sys_ctrl.h"
#include "../interfaces/board-i2c.h"
#include "../system.h"
#include "../inc/hw_aon_event.h"

// RTC Handler
#include "../inc/hw_ints.h"
#include "../inc/hw_nvic.h"
#include "../driverLib/aon_rtc.h"
#include "../driverLib/sys_ctrl.h"

// GPIO Handler
#include "../../board.h"
#include "../driverLib/gpio.h"

// RFC Handler
#include "../inc/hw_rfc_dbell.h"
#include "../inc/hw_rfc_pwr.h"
#include "../inc/hw_fcfg1.h"
#include "../radio_files/rfc_api/common_cmd.h"
#include "../radio_files/rfc_api/ble_cmd.h"
#include "../radio_files/rfc_api/mailbox.h"
#include "../radio_files/patches/ble/apply_patch.h"
// #include "../radio_files/overrides/ble_overrides.h"  // linker-fehler, wenn zweimal included
#include "../driverLib/prcm.h"
#include "../../radio.h"
#include "../../system.h"
#include "../../config.h"  // set global variables


// globale variables:
// declared in main. used here and in radio.c
// ------------------------------------------

// Button input
bool g_button_pressed;

// RTC Wake up
long g_current_wake_up_time;

// RTC Speed measuring
uint32_t g_timestamp1, g_timestamp2, g_timeDiff;
bool g_measurement_done;

// Pressure Sensor
bool g_pressure_set;					// pressure sensor state
uint16_t g_pressure;

// RFChip  Send BLE data
volatile bool rfBootDone;
volatile bool rfSetupDone;
volatile bool rfAdvertisingDone;

//*****************************************************************************
//
//! Forward declaration of the reset ISR and the default fault handlers.
//
//*****************************************************************************
void        ResetISR( void );
static void NmiSR( void );
static void FaultISR( void );
static void IntDefaultHandler( void );

extern int  main(void);



static void MPUFaultIntHandler( void );
extern void MPUFaultIntHandler( void );
static void BusFaultIntHandler( void );
extern void BusFaultIntHandler( void );
static void UsageFaultIntHandler( void );
extern void UsageFaultIntHandler( void );
static void SVCallIntHandler( void );
extern void SVCallIntHandler( void );
static void DebugMonIntHandler( void );
extern void DebugMonIntHandler( void );
static void PendSVIntHandler( void );
extern void PendSVIntHandler( void );
static void SysTickIntHandler( void );
extern void SysTickIntHandler( void );
static void GPIOIntHandler( void );
static void GPIOIntHandler( void );
static void GPIOIntHandler( void );
extern void GPIOIntHandler( void );
static void I2CIntHandler( void );
extern void I2CIntHandler( void );
static void RFCCPE1IntHandler( void );
extern void RFCCPE1IntHandler( void );
static void AONIntHandler( void );
extern void AONIntHandler( void );
static void AONRTCIntHandler( void );
extern void AONRTCIntHandler( void );
static void UART0IntHandler( void );
extern void UART0IntHandler( void );
static void AUXSWEvent0IntHandler( void );
extern void AUXSWEvent0IntHandler( void );
static void SSI0IntHandler( void );
extern void SSI0IntHandler( void );
static void SSI1IntHandler( void );
extern void SSI1IntHandler( void );
static void RFCCPE0IntHandler( void );
extern void RFCCPE0IntHandler( void );
static void RFCHardwareIntHandler( void );
extern void RFCHardwareIntHandler( void );
static void RFCCmdAckIntHandler( void );
extern void RFCCmdAckIntHandler( void );
static void I2SIntHandler( void );
extern void I2SIntHandler( void );
static void AUXSWEvent1IntHandler( void );
extern void AUXSWEvent1IntHandler( void );
static void WatchdogIntHandler( void );
extern void WatchdogIntHandler( void );
static void Timer0AIntHandler( void );
extern void Timer0AIntHandler( void );
static void Timer0BIntHandler( void );
extern void Timer0BIntHandler( void );
static void Timer1AIntHandler( void );
extern void Timer1AIntHandler( void );
static void Timer1BIntHandler( void );
extern void Timer1BIntHandler( void );
static void Timer2AIntHandler( void );
extern void Timer2AIntHandler( void );
static void Timer2BIntHandler( void );
extern void Timer2BIntHandler( void );
static void Timer3AIntHandler( void );
extern void Timer3AIntHandler( void );
static void Timer3BIntHandler( void );
extern void Timer3BIntHandler( void );
static void CryptoIntHandler( void );
extern void CryptoIntHandler( void );
static void uDMAIntHandler( void );
extern void uDMAIntHandler( void );
static void uDMAErrIntHandler( void );
extern void uDMAErrIntHandler( void );
static void FlashIntHandler( void );
extern void FlashIntHandler( void );
static void SWEvent0IntHandler( void );
extern void SWEvent0IntHandler( void );
static void AUXCombEventIntHandler( void );
extern void AUXCombEventIntHandler( void );
static void AONProgIntHandler( void );
extern void AONProgIntHandler( void );
static void DynProgIntHandler( void );
extern void DynProgIntHandler( void );
static void AUXCompAIntHandler( void );
extern void AUXCompAIntHandler( void );
static void AUXADCIntHandler( void );
extern void AUXADCIntHandler( void );
static void TRNGIntHandler( void );
extern void TRNGIntHandler( void );



//*****************************************************************************
//
//! The entry point for the application startup code and device trim fxn.
//
//*****************************************************************************
extern void _c_int00(void);
extern void trimDevice(void);


//*****************************************************************************
//
// CCS: Linker variable that marks the top of the stack.
//
//*****************************************************************************
extern unsigned long __STACK_END;


//! The vector table. Note that the proper constructs must be placed on this to
//! ensure that it ends up at physical address 0x0000.0000 or at the start of
//! the program if located at a start address other than 0.
//
//*****************************************************************************
#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((unsigned long)&__STACK_END),
                                            // The initial stack pointer
    ResetISR,                               // The reset handler
    NmiSR,                                  // The NMI handler
    FaultISR,                               // The hard fault handler
    MPUFaultIntHandler,                     // The MPU fault handler
    BusFaultIntHandler,                     // The bus fault handler
    UsageFaultIntHandler,                   // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    SVCallIntHandler,                       // SVCall handler
    DebugMonIntHandler,                     // Debug monitor handler
    0,                                      // Reserved
    PendSVIntHandler,                       // The PendSV handler
    SysTickIntHandler,                      // The SysTick handler
    GPIOIntHandler,                         // AON edge detect
    I2CIntHandler,                          // I2C
    RFCCPE1IntHandler,                      // RF Core Command & Packet Engine 1
    AONIntHandler,                          // AON SpiSplave Rx, Tx and CS
    AONRTCIntHandler,                       // AON RTC
    UART0IntHandler,                        // UART0 Rx and Tx
    AUXSWEvent0IntHandler,                  // AUX software event 0
    SSI0IntHandler,                         // SSI0 Rx and Tx
    SSI1IntHandler,                         // SSI1 Rx and Tx
    RFCCPE0IntHandler,                      // RF Core Command & Packet Engine 0
    RFCHardwareIntHandler,                  // RF Core Hardware
    RFCCmdAckIntHandler,                    // RF Core Command Acknowledge
    I2SIntHandler,                          // I2S
    AUXSWEvent1IntHandler,                  // AUX software event 1
    WatchdogIntHandler,                     // Watchdog timer
    Timer0AIntHandler,                      // Timer 0 subtimer A
    Timer0BIntHandler,                      // Timer 0 subtimer B
    Timer1AIntHandler,                      // Timer 1 subtimer A
    Timer1BIntHandler,                      // Timer 1 subtimer B
    Timer2AIntHandler,                      // Timer 2 subtimer A
    Timer2BIntHandler,                      // Timer 2 subtimer B
    Timer3AIntHandler,                      // Timer 3 subtimer A
    Timer3BIntHandler,                      // Timer 3 subtimer B
    CryptoIntHandler,                       // Crypto Core Result available
    uDMAIntHandler,                         // uDMA Software
    uDMAErrIntHandler,                      // uDMA Error
    FlashIntHandler,                        // Flash controller
    SWEvent0IntHandler,                     // Software Event 0
    AUXCombEventIntHandler,                 // AUX combined event
    AONProgIntHandler,                      // AON programmable 0
    DynProgIntHandler,                      // Dynamic Programmable interrupt
                                            // source (Default: PRCM)
    AUXCompAIntHandler,                     // AUX Comparator A
    AUXADCIntHandler,                       // AUX ADC new sample or ADC DMA
                                            // done, ADC underflow, ADC overflow
    TRNGIntHandler                          // TRNG event
};

//*****************************************************************************
//
// 2 active interrupt in RTC-handler:
//
// RTC0 = Wake up timer
// RTC2 = Speed measuring
//
//*****************************************************************************

void AONRTCIntHandler(void) {

	// Wake up Timer
	// --------------
	if(AONRTCEventGet(AON_RTC_CH0)){
		AONRTCEventClear(AON_RTC_CH0);		 // Clear RTC 0 event flag
		AONRTCCompareValueSet(AON_RTC_CH0, AONRTCCompareValueGet(AON_RTC_CH0)+(g_current_wake_up_time));  // set new wake up time

	}
	// Speed measurement Timer
	// -----------------------
	if(AONRTCEventGet(AON_RTC_CH2)){
		AONRTCEventClear(AON_RTC_CH2);		// Clear RTC 2 event flag
	}
	int debug = 2;
}



void GPIOIntHandler(void){

	uint32_t pin_mask = 0;
	IntDisable(INT_EDGE_DETECT);				// block temporary new interrupts

	// power on
	powerEnablePeriph();
	powerEnableGPIOClockRunMode();
	while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH) != PRCM_DOMAIN_POWER_ON));


	pin_mask = (HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) & GPIO_PIN_MASK);				/* Read interrupt flags */

	// wait because Edge is bouncing for approx. 3 ms
	CPUdelay(10000); 						// 10 ms								// 40 km/h  => alle 90 ms eine halbe Umdrehung
																					// 80 km/h  => 47 ms
																					// 100 km/h => 37 ms
	// handling ********************************

	// Reed-Pin for speed measuring
	// ----------------------------
	// if( IOCIntStatus(IOID_25) )
	if(pin_mask == GPIO_DOUT31_0_DIO25 ){
		IOCIntClear(IOID_25);
		IntPendClear(INT_EDGE_DETECT);
		// set timestamp
		if(g_timestamp1 == 0){
			// Get the first Interrupt Time
			g_timestamp1 = AONRTCCurrentCompareValueGet(); // AONRTCCurrentSubSecValueGet();
			int debug_1 = 5;
		} else {
			// Get the second Interrupt Time
			g_timestamp2 = AONRTCCurrentCompareValueGet(); // AONRTCCurrentSubSecValueGet();
			int debug = 2;
			g_measurement_done = true;
			AONRTCReset();
		}
	}

	// Button
	// -----------------------------------
	if(pin_mask == GPIO_DOUT31_0_DIO4 ){
		IOCIntClear(IOID_4);
		IntPendClear(INT_EDGE_DETECT);
		HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) = pin_mask;		/* Clear the interrupt flags */																	/* Clear pending interrupts */
		g_button_pressed = true;
	}

	/* Clear the interrupt flags */
	HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) = pin_mask;

	// power down after wake up
	powerDisablePeriph();							// Power off IOC domain
	HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0; // Disable clock for GPIO in CPU run mode
	HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;  // Load clock settings

	// from Dario
	__asm(" nop");
	__asm(" nop");
	__asm(" nop");
	__asm(" nop");
	__asm(" nop");
	__asm(" nop");

	// get interrupts free
	IntEnable(INT_EDGE_DETECT);
}



//Radio CPE ch 1 interrupt - used for BOOT_DONE ISR (by default on CH1)
void RFCCPE1IntHandler(void) {
  //Clear all RF Core ISR flags and wair until done
  do {
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_BOOT_DONE_M;
  }
  while(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG)  & RFC_DBELL_RFCPEIFG_BOOT_DONE_M);

  rfBootDone = 1;
}


//Radio CPE ch 0 interrupt. Used for CMD_DONE and LAST_CMD_DONE (by default on CH0)
//
void RFCCPE0IntHandler(void) {

  uint32_t interrupts = HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG);

  if(interrupts & RFC_DBELL_RFCPEIFG_COMMAND_DONE_M) {
    rfSetupDone = 1;
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) &= ~RFC_DBELL_RFCPEIEN_COMMAND_DONE_M;

    do {
      HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_COMMAND_DONE_M;
    }
    while(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) & RFC_DBELL_RFCPEIFG_COMMAND_DONE_M);
    }

  if(interrupts & RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M) {
    rfAdvertisingDone = 1;
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) &= ~RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

    do {
      HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M;
    }
    while(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIFG) & RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M);
    }


}











//*****************************************************************************
//
//! This is the code that gets called when the processor first starts execution
//! following a reset event. Only the absolutely necessary set is performed,
//! after which the application supplied entry() routine is called. Any fancy
//! actions (such as making decisions based on the reset cause register, and
//! resetting the bits in that register) are left solely in the hands of the
//! application.
//
//*****************************************************************************
void
ResetISR(void)
{
    //
    // Final trim of device
    //
    trimDevice();

    //
    // Jump to the CCS C Initialization Routine.
    //
    __asm("    .global _c_int00\n"
            "    b.w     _c_int00");

    //
    // If we ever return signal Error
    //
    FaultISR();
}

//*****************************************************************************
//
//! This is the code that gets called when the processor receives a NMI. This
//! simply enters an infinite loop, preserving the system state for examination
//! by a debugger.
//
//*****************************************************************************
static void
NmiSR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
//! This is the code that gets called when the processor receives a fault
//! interrupt. This simply enters an infinite loop, preserving the system state
//! for examination by a debugger.
//
//*****************************************************************************
static void
FaultISR(void)
{
    //
    // Enter an infinite loop.
    //
    while(1)
    {
    }
}


//*****************************************************************************
//
//! This is the code that gets called when the processor receives an unexpected
//! interrupt. This simply enters an infinite loop, preserving the system state
//! for examination by a debugger.
//
//*****************************************************************************
static void
IntDefaultHandler(void)
{
    //
    // Go into an infinite loop.
    //
    while(1)
    {
    }
}

static void MPUFaultIntHandler( void ){ while(1) {}}
static void BusFaultIntHandler( void ){ while(1) {}}
static void UsageFaultIntHandler( void ){ while(1) {}}
static void SVCallIntHandler( void ){ while(1) {}}
static void DebugMonIntHandler( void ){ while(1) {}}
static void PendSVIntHandler( void ){ while(1) {}}
static void SysTickIntHandler( void ){ while(1) {}}
//static void GPIOIntHandler( void ){ while(1) {}}
static void I2CIntHandler( void ){ while(1) {}}
static void AONIntHandler( void ){ while(1) {}}

static void UART0IntHandler( void ){ while(1) {}}
static void AUXSWEvent0IntHandler( void ){ while(1) {}}
static void SSI0IntHandler( void ){ while(1) {}}
static void SSI1IntHandler( void ){ while(1) {}}
static void RFCHardwareIntHandler( void ){ while(1) {}}
static void RFCCmdAckIntHandler( void ){ while(1) {}}
static void I2SIntHandler( void ){ while(1) {}}
static void AUXSWEvent1IntHandler( void ){ while(1) {}}
static void WatchdogIntHandler( void ){ while(1) {}}
static void Timer0AIntHandler( void ){ while(1) {}}
static void Timer0BIntHandler( void ){ while(1) {}}
static void Timer1AIntHandler( void ){ while(1) {}}
static void Timer1BIntHandler( void ){ while(1) {}}
static void Timer2AIntHandler( void ){ while(1) {}}
static void Timer2BIntHandler( void ){ while(1) {}}
static void Timer3AIntHandler( void ){ while(1) {}}
static void Timer3BIntHandler( void ){ while(1) {}}
static void CryptoIntHandler( void ){ while(1) {}}
static void uDMAIntHandler( void ){ while(1) {}}
static void uDMAErrIntHandler( void ){ while(1) {}}
static void FlashIntHandler( void ){ while(1) {}}
static void SWEvent0IntHandler( void ){ while(1) {}}
static void AUXCombEventIntHandler( void ){ while(1) {}}
static void AONProgIntHandler( void ){ while(1) {}}
static void DynProgIntHandler( void ){ while(1) {}}
static void AUXCompAIntHandler( void ){ while(1) {}}
static void AUXADCIntHandler( void ){ while(1) {}}
static void TRNGIntHandler( void ){ while(1) {}}

