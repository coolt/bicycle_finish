/*
 * Author:			Daniel Studer
 * Date:			2015-11
 * File:			em8500-pwr-mgnt.h
 *
 * This File is to access the EM8500 from the SensorTag over SPI
 *
 */

#ifndef EM8500_PWR_MGNT_H_
#define EM8500_PWR_MGNT_H_

// --- Includes --------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cc26xxware_2_22_00_16101/driverLib/ioc.h"
#include "cc26xxware_2_22_00_16101/driverLib/gpio.h"

// --- Defines ---------------------------------------------------------------------

// Pins on CC2650 connected to EM8500
#define EM_SPI_CLK		IOID_23
#define EM_SPI_CS		IOID_24
#define EM_SPI_CS_GPIO	GPIO_PIN_24

// SPI Commands to write/read to Register
#define SPI_READ		0x80
#define SPI_WRITE		0x00

// EM8500 Registers
#define REG_PWR_MGT		0x19
#define VSUP_SLEEP		0x01

// --- Prototypes -------------------------------------------------------------------

void em8500_Init(void);
void em8500_DisableVSUP(void);

#endif /* EM8500_PWR_MGNT_H_ */
