/*
 * Author:			Daniel Studer
 * Date:			2015-11
 * File:			em8500-pwr-mgnt.h
 *
 * This File is to access the EM8500 from the SensorTag over SPI
 *
 */

// --- Includes --------------------------------------------------------------------

#include "em8500-pwr-mgnt.h"
#include "ti-lib.h"
#include "board-spi.h"
#include "cc26xxware_2_22_00_16101/driverLib/ioc.h"
#include "cc26xxware_2_22_00_16101/driverLib/gpio.h"
#include "system.h"

#include <stdint.h>
#include <stdbool.h>

// --- local functions -------------------------------------------------------------

/**
 * \brief SPI Chip Select Line activate
 */
static void select(void)
{
	GPIOPinWrite(EM_SPI_CS_GPIO, 1);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief SPI Chip Select Line deactivate
 */
static void deselect(void)
{
	GPIOPinWrite(EM_SPI_CS_GPIO, 0);
}

// --- global functions -------------------------------------------------------------
/**
 * \brief Initialize the Chipselect Line for accessing the EM8500
 */
void em8500_Init(void){
	// GPIO pin configuration
	IOCPortConfigureSet(EM_SPI_CS, IOC_PORT_GPIO, IOC_STD_OUTPUT);
	// Set Pin as output
	GPIODirModeSet(EM_SPI_CS_GPIO, GPIO_DIR_MODE_OUT);

	// Default output to clear chip select
	deselect();
}

/*---------------------------------------------------------------------------*/
/**
 * \brief 	Call this function to enable the vsup_sleep Bit on EM8500. The EM8500
 * 			will disable VSUP und will enable it after the programmed sleep-delay
 */
void em8500_DisableVSUP(void){
	uint8_t reg, data;
	bool success;

	// Enable Peripheral to access the Chipselect Line
	powerEnablePeriph();
	powerEnableGPIOClockRunMode();

	/* Wait for domains to power on */
	while((PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH)
			!= PRCM_DOMAIN_POWER_ON));

	// register the SPI
	board_spi_open(400000, EM_SPI_CLK);

	reg = (SPI_WRITE | REG_PWR_MGT);
	data = VSUP_SLEEP;

	// Select the Chip and write Data
	select();
	success = board_spi_write(&reg, sizeof(reg));
	success = board_spi_write(&data, sizeof(data));

	// Release the Chip
	deselect();

	// Release the SPI
	board_spi_close();

	// Disable Peripheral
	powerDisablePeriph();
	// Disable clock for GPIO in CPU run mode
	HWREGBITW(PRCM_BASE + PRCM_O_GPIOCLKGR, PRCM_GPIOCLKGR_CLK_EN_BITN) = 0;
	// Load clock settings
	HWREGBITW(PRCM_BASE + PRCM_O_CLKLOADCTL, PRCM_CLKLOADCTL_LOAD_BITN) = 1;
}
