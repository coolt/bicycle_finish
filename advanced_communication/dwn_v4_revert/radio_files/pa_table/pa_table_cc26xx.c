/*
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "inc/hw_types.h"
// #include "radio_defs.h"

const txPowerTable_t txPower_cc26xx_diff_intbias[] = {
 { .txPowerdBm =   5,   .IB = 0x31,  .GC = 0x0 },
 { .txPowerdBm =   4,   .IB = 0x25,  .GC = 0x0 },
 { .txPowerdBm =   3,   .IB = 0x31,  .GC = 0x1 },
 { .txPowerdBm =   2,   .IB = 0x2A,  .GC = 0x1 },
 { .txPowerdBm =   1,   .IB = 0x25,  .GC = 0x1 },
 { .txPowerdBm =   0,   .IB = 0x26,  .GC = 0x3 },
 { .txPowerdBm =  -3,   .IB = 0x1C,  .GC = 0x3 },
 { .txPowerdBm =  -6,   .IB = 0x16,  .GC = 0x3 },
 { .txPowerdBm =  -9,   .IB = 0x11,  .GC = 0x3 },
 { .txPowerdBm = -12,   .IB = 0x0E,  .GC = 0x3 },
 { .txPowerdBm = -15,   .IB = 0x0B,  .GC = 0x3 },
 { .txPowerdBm = -18,   .IB = 0x08,  .GC = 0x3 },
 { .txPowerdBm = -21,   .IB = 0x06,  .GC = 0x3 },
};

const uint8_t txPower_cc26xx_diff_intbias_len =  sizeof(txPower_cc26xx_diff_intbias) /
                                                 sizeof(txPowerTable_t);
