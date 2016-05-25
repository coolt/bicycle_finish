//------------------------------------------------------------------------------
// TI Confidential - NDA Restrictions
//
// Copyright (c) 2014 Texas Instruments, Inc.
//
//    This is an unpublished work created in the year stated above.
//    Texas Instruments owns all rights in and to this work and
//    intends to maintain and protect it as an unpublished copyright.
//    In the event of either inadvertent or deliberate publication,
//    the above stated date shall be treated as the year of first
//    publication. In the event of such publication, Texas Instruments
//    intends to enforce its rights in the work under the copyright
//    laws as a published work.
//
//------------------------------------------------------------------------------
//
// This file is auto-generated and should not be edited 
// Generated Thu Oct 30 15:34:40 2014 using the following files:
// /vobs/cc26xxFwDev/radio_domain/patch/pg2/topsm_ble/rfe/src/rfe_ram_bank.asm@@/main/1     Rule: /main/LATEST
// /vobs/cc26xxFwDev/radio_domain/rfe/src/dbg.asm@@/main/1  Rule: CC26_RF_ROM_FW_RFE--DEV--1.0--N.A.00.18--2014.04.01
// /vobs/cc26xxFwDev/radio_domain/patch/pg2/topsm_ble/rfe/src/rfe_commonlib.asm@@/main/1    Rule: /main/LATEST
// /vobs/cc26xxFwDev/radio_domain/rfe/doc/rfeadiconf.txt@@/main/10          Rule: CC26_RF_ROM_FW_RFE--DEV--1.0--N.A.00.18--2014.04.01
// /vobs/cc26xxIpDev/modules/cc26_rfcore/doc/RFEregs.txt@@/main/26          Rule: /main/LATEST
// /vobs/cc26xxIpDev/modules/cc26_rfcore/doc/RFEADIregs.txt@@/main/12       Rule: /main/LATEST
//

// This file implements patches for the RFE on CC26xx
// It should only be included from ONE source file to avoid duplicated constant arrays


#ifndef _APPLY_BLE_RFE_PATCH_H
#define _APPLY_BLE_RFE_PATCH_H

#include <stdint.h>

#ifndef RFE_PATCH_TYPE
#define RFE_PATCH_TYPE static const uint32_t
#endif

#ifndef RFE_ADDR_TYPE
#define RFE_ADDR_TYPE static const uint16_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static inline
#endif

#ifndef RFC_RFERAM_BASE
#define RFC_RFERAM_BASE 0x2100C000
#endif
#define BLE_RFE_NUM_COPY 0x0276

RFE_ADDR_TYPE bleRfePatchAdd[3] = {
   2,
   65,
   280
};

RFE_PATCH_TYPE bleRfePatchDat[3] = {
   0x00050006,
   0x26514084,
   0xc0f21000
};

// Note that this function does NOT call the DUMP API function of the TOPsm,
// the ROM contents are assumed to be present in RAM
PATCH_FUN_SPEC void enterBleRfePatch(void)
{
   uint32_t *pRfeRam = (uint32_t *) RFC_RFERAM_BASE;
   for (int k = 0; k < 3; k++) {
      pRfeRam[bleRfePatchAdd[k]] = bleRfePatchDat[k];
   }
}

#endif
