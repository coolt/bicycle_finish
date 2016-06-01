/******************************************************************************
*  Filename:       startup_ccs.c
*  Revised:        $Date: 2015-05-19 15:09:27 +0200 (ti, 19 mai 2015) $
*  Revision:       $Revision: 15737 $
*
*  Description:    Startup code for CC26xx PG2 device family for use with CCS.
*
*  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include <inc/hw_types.h>


//*****************************************************************************
//
//! Forward declaration of the reset ISR and the default fault handlers.
//
//*****************************************************************************
void        ResetISR( void );
static void NmiSR( void );
static void FaultISR( void );

static void IntDefaultHandler( void );
extern void IntDefaultHandler( void );

extern int  main(void);

// static for not used handler
// implemented handler only extern
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
//static void GPIOIntHandler( void );
extern void GPIOIntHandler( void );
static void I2CIntHandler( void );
extern void I2CIntHandler( void );
//static void RFCCPE1IntHandler( void );
extern void RFCCPE1IntHandler( void );
static void AONIntHandler( void );
extern void AONIntHandler( void );
//static void AONRTCIntHandler( void );
extern void AONRTCIntHandler( void );
static void UART0IntHandler( void );
extern void UART0IntHandler( void );
static void AUXSWEvent0IntHandler( void );
extern void AUXSWEvent0IntHandler( void );
static void SSI0IntHandler( void );
extern void SSI0IntHandler( void );
static void SSI1IntHandler( void );
extern void SSI1IntHandler( void );
//static void RFCCPE0IntHandler( void );
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
//static void GPIOIntHandler( void ){ while(1) {}}  // see main
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
