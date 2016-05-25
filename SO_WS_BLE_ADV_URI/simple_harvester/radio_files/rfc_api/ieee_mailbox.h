//------------------------------------------------------------------------------
// TI Confidential - NDA Restrictions
//
// Copyright (c) 2011 Texas Instruments, Inc.
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
/// \file            ieee_mailbox.h
/// \brief           Definitions for IEEE 802.15.4 interface
///
/// \author          Low Power RF Wireless Business Unit
///                  Helge Coward (h.coward@ti.com)
///
/// \date            Thu Nov 24 09:00:42 CET 2011
///
//-----------------------------------------------------------------------------

#ifndef _IEEE_MAILBOX_H
#define _IEEE_MAILBOX_H

#include "mailbox.h"

/// \name CPE interrupt definitions for IEEE 802.15.4
/// Interrupt masks for the CPE interrupt in RDBELL. These are new names for interrupts in mailbox.h,
/// used for compartibility with previous versions with separate interrupt numbers.
///@{
#define IRQN_IEEE_BG_COMMAND_SUSPENDED IRQN_BG_COMMAND_SUSPENDED
#define IRQN_IEEE_TX_FRAME             IRQN_TX_DONE
#define IRQN_IEEE_TX_ACK               IRQN_TX_ACK

#define IRQN_IEEE_RX_FRAME             IRQN_RX_OK
#define IRQN_IEEE_RX_NOK               IRQN_RX_NOK
#define IRQN_IEEE_RX_IGNORED           IRQN_RX_IGNORED
#define IRQN_IEEE_RX_BUF_FULL          IRQN_RX_BUF_FULL
#define IRQN_IEEE_RX_ENTRY_DONE        IRQN_RX_ENTRY_DONE

#define IRQ_IEEE_BG_COMMAND_SUSPENDED  (1U << IRQN_IEEE_BG_COMMAND_SUSPENDED)
#define IRQ_IEEE_TX_FRAME              (1U << IRQN_IEEE_TX_FRAME)
#define IRQ_IEEE_TX_ACK                (1U << IRQN_IEEE_TX_ACK)
#define IRQ_IEEE_RX_FRAME              (1U << IRQN_IEEE_RX_FRAME)
#define IRQ_IEEE_RX_NOK                (1U << IRQN_IEEE_RX_NOK)
#define IRQ_IEEE_RX_IGNORED            (1U << IRQN_IEEE_RX_IGNORED)
#define IRQ_IEEE_RX_BUF_FULL           (1U << IRQN_IEEE_RX_BUF_FULL)
#define IRQ_IEEE_RX_ENTRY_DONE         (1U << IRQN_IEEE_RX_ENTRY_DONE)
///@}



/// \name Radio operation status
/// Radio operation status format:
/// Bits 15:12: Protocol
///             0010: IEEE 802.15.4
/// Bits 11:10: Type
///             00: Not finished
///             01: Done successfully
///             10: Done with error
/// Bits 9:0:   Identifier

/// \name Operation not finished
///@{
#define IEEE_SUSPENDED          0x2001  ///< Operation suspended
///@}
/// \name Operation finished normally
///@{
#define IEEE_DONE_OK            0x2400  ///< Operation ended normally
#define IEEE_DONE_BUSY          0x2401  ///< CSMA-CA operation ended with failure
#define IEEE_DONE_STOPPED       0x2402  ///< Operation stopped after stop command
#define IEEE_DONE_ACK           0x2403  ///< ACK packet received with pending data bit cleared
#define IEEE_DONE_ACKPEND       0x2404  ///< ACK packet received with pending data bit set
#define IEEE_DONE_TIMEOUT       0x2405  ///< Operation ended due to timeout
#define IEEE_DONE_BGEND         0x2406  ///< FG operation ended because necessary background level
                                        ///< operation ended 
#define IEEE_DONE_ABORT         0x2407  ///< Operation aborted by command
///@}
/// \name Operation finished with error
///@{
#define IEEE_ERROR_PAR          0x2800  ///< Illegal parameter
#define IEEE_ERROR_NO_SETUP     0x2801  ///< Operation using Rx or Tx attemted when not in 15.4 mode
#define IEEE_ERROR_NO_FS        0x2802  ///< Operation using Rx or Tx attemted without frequency synth configured
#define IEEE_ERROR_SYNTH_PROG   0x2803  ///< Synthesizer programming failed to complete on time
#define IEEE_ERROR_RXOVF        0x2804  ///< Receiver overflowed during operation
#define IEEE_ERROR_TXUNF        0x2805  ///< Transmitter underflowed during operation
///@}
///@}

#endif
