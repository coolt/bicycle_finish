//------------------------------------------------------------------------------
// TI Confidential - NDA Restrictions
//
// Copyright (c) 2013 Texas Instruments, Inc.
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
///
/// \file            ant_mailbox.h
/// \brief           Definitions for ANT mode radio interface
///
/// \author          Low Power RF Wireless Business Unit
///                  Helge Coward (h.coward@ti.com)
///
/// \date            Wed Dec  4 16:07:09 CET 2013
///
//-----------------------------------------------------------------------------

#ifndef _ANT_MAILBOX_H
#define _ANT_MAILBOX_H



/// \name Radio operation status
/// Radio operation status format:
/// Bits 15:12: Protocol
///             0100: ANT
/// Bits 11:10: Type
///             00: Not finished
///             01: Done successfully
///             10: Done with error
/// Bit  9:     N/A
/// Bits 8:6:   Sub-type:
///             000: ANT
/// Bits 5:0:   Identifier

/// \name Operation finished normally
///@{
#define ANT_DONE_OK            0x4400  ///< Operation ended normally
#define ANT_DONE_RXTIMEOUT     0x4401  ///< Operation stopped after end trigger while waiting for sync
#define ANT_DONE_RXERR         0x4402  ///< Operation ended after CRC error
#define ANT_DONE_ENDED         0x4403  ///< Operation stopped after end trigger during reception
#define ANT_DONE_STOPPED       0x4404  ///< Operation stopped after stop command
#define ANT_DONE_ABORT         0x4405  ///< Operation aborted by abort command
///@}
/// \name Operation finished with error
///@{
#define ANT_ERROR_PAR          0x4800  ///< Illegal parameter
#define ANT_ERROR_RXBUF        0x4801  ///< No available Rx buffer at the start of a packet
#define ANT_ERROR_NO_SETUP     0x4802  ///< Radio was not set up in a compatible mode
#define ANT_ERROR_NO_FS        0x4803  ///< Synth was not programmed when running Rx or Tx
#define ANT_ERROR_RXOVF        0x4804  ///< Rx overflow observed during operation
#define ANT_ERROR_TXUNF        0x4805  ///< Tx underflow observed during operation
///@}
///@}

#endif
