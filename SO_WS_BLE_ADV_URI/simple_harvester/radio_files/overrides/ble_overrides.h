//*****************************************************************************
//! @file       ble_overrides.c
//! @brief      Recommended overrides for CC26xx PG2.1
//!
//! Revised     $ $
//! Revision    $ $
//
//  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/

#include <stdint.h>


// Recommended overrides for Bluetooth Low Energy, differential mode

//
// Modified from original to remove the "run RFE from RAM override" since it is not
// used by the broadcaster (RFE patch only needed for RX)
//

uint32_t bleDifferentialOverrides[] = {
  0x00354038, // Synth: Set RTRIM (POTAILRESTRIM) to 5
  0x4001402D, // Synth: Correct CKVD latency setting (address)
  0x00608402, // Synth: Correct CKVD latency setting (value)
  0x4001405D, // Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address)
  0x1801F800, // Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value)
  0x000784A3, // Synth: Set FREF = 3.43 MHz (24 MHz / 7)
  0xA47E0583, // Synth: Set loop bandwidth after lock to 80 kHz (K2)
  0xEAE00603, // Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB)
  0x00010623, // Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB)
  0x00456088, // Adjust AGC reference level
  0x013800C3, // Use enhanced BLE shape
  0xFFFFFFFF, // End of override list
};


// Recommended overrides for Bluetooth Low Energy, single-ended mode
uint32_t bleSingleOverrides[] = {
  0x00354038, // Synth: Set RTRIM (POTAILRESTRIM) to 5
  0x4001402D, // Synth: Correct CKVD latency setting (address)
  0x00608402, // Synth: Correct CKVD latency setting (value)
  0x4001405D, // Synth: Set ANADIV DIV_BIAS_MODE to PG1 (address)
  0x1801F800, // Synth: Set ANADIV DIV_BIAS_MODE to PG1 (value)
  0x000784A3, // Synth: Set FREF = 3.43 MHz (24 MHz / 7)
  0xA47E0583, // Synth: Set loop bandwidth after lock to 80 kHz (K2)
  0xEAE00603, // Synth: Set loop bandwidth after lock to 80 kHz (K3, LSB)
  0x00010623, // Synth: Set loop bandwidth after lock to 80 kHz (K3, MSB)
  0x000288A3, // Single ended mode only: Adjust RSSI offset by 2 dB
  0x00456088, // Adjust AGC reference level
  0x013800C3, // Use enhanced BLE shape
  0xFFFFFFFF, // End of override list
};

