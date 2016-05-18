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
/// \file            hs_mailbox.h
/// \brief           Definitions for high-speed mode radio interface
///
/// \author          Low Power RF Wireless Business Unit
///                  Helge Coward (h.coward@ti.com)
///
/// \date            Mon Nov  4 16:03:22 CET 2013
///
//-----------------------------------------------------------------------------

#ifndef _HS_MAILBOX_H
#define _HS_MAILBOX_H



/// \name Radio operation status
/// Radio operation status format:
/// Bits 15:12: Protocol
///             0011: Proprietary
/// Bits 11:10: Type
///             00: Not finished
///             01: Done successfully
///             10: Done with error
/// Bit  9:     N/A
/// Bits 8:6:   Sub-type:
///             001: High-speed
/// Bits 9:0:   Identifier

/// \name Operation finished normally
///@{
#define HS_DONE_OK            0x3440  ///< Operation ended normally
#define HS_DONE_RXTIMEOUT     0x3441  ///< Operation stopped after end trigger while waiting for sync
#define HS_DONE_RXERR         0x3442  ///< Operation ended after CRC error
#define HS_DONE_TXBUF         0x3443  ///< Tx queue was empty at start of operation
#define HS_DONE_ENDED         0x3444  ///< Operation stopped after end trigger during reception
#define HS_DONE_STOPPED       0x3445  ///< Operation stopped after stop command
#define HS_DONE_ABORT         0x3446  ///< Operation aborted by abort command
///@}
/// \name Operation finished with error
///@{
#define HS_ERROR_PAR          0x3840  ///< Illegal parameter
#define HS_ERROR_RXBUF        0x3841  ///< No available Rx buffer at the start of a packet
#define HS_ERROR_NO_SETUP     0x3842  ///< Radio was not set up in a compatible mode
#define HS_ERROR_NO_FS        0x3843  ///< Synth was not programmed when running Rx or Tx
#define HS_ERROR_RXOVF        0x3844  ///< Rx overflow observed during operation
#define HS_ERROR_TXUNF        0x3845  ///< Tx underflow observed during operation
///@}
///@}

#endif
