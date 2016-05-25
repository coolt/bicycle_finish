/*

 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-spi
 * @{
 *
 * \file
 * Board-specific SPI driver for the Sensortag-CC26xx
 */
/*---------------------------------------------------------------------------*/
#include "../interfaces/board-spi.h"

#include "ti-lib.h"
#include <stdbool.h>

#define BOARD_IOID_FLASH_CS       		IOID_14
#define BOARD_IOID_SPI_MISO       	  	IOID_18
#define BOARD_IOID_SPI_MOSI      	  	IOID_19
#define BOARD_FLASH_CS            		(1 << BOARD_IOID_FLASH_CS)
#define BOARD_IOID_SPI_CLK_FLASH  		IOID_17

/*---------------------------------------------------------------------------*/
static bool accessible(void)
{
  /* First, check the PD */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
     != PRCM_DOMAIN_POWER_ON) {
    return false;
  }

  /* Then check the 'run mode' clock gate */
  if(!(HWREG(PRCM_BASE + PRCM_O_SSICLKGR) & PRCM_SSICLKGR_CLK_EN_SSI0)) {
    return false;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
bool board_spi_write(const uint8_t *buf, size_t len)
{
  if(accessible() == false) {
    return false;
  }

  while(len > 0) {
    uint32_t ul;

    ti_lib_ssi_data_put(SSI0_BASE, *buf);
    ti_lib_rom_ssi_data_get(SSI0_BASE, &ul);
    len--;
    buf++;
  }

  return true;
}
/*---------------------------------------------------------------------------*/
bool board_spi_read(uint8_t *buf, size_t len)
{
  if(accessible() == false) {
    return false;
  }

  while(len > 0) {
    uint32_t ul;

    if(!ti_lib_rom_ssi_data_put_non_blocking(SSI0_BASE, 0)) {
      /* Error */
      return false;
    }
    ti_lib_rom_ssi_data_get(SSI0_BASE, &ul);
    *buf = (uint8_t)ul;
    len--;
    buf++;
  }
  return true;
}
/*---------------------------------------------------------------------------*/
void board_spi_flush()
{
  if(accessible() == false) {
    return;
  }

  uint32_t ul;
  while(ti_lib_rom_ssi_data_get_non_blocking(SSI0_BASE, &ul));
}
/*---------------------------------------------------------------------------*/
void board_spi_open(uint32_t bit_rate, uint32_t clk_pin)
{
  uint32_t buf;

  /* First, make sure the SERIAL PD is on */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
        != PRCM_DOMAIN_POWER_ON));

  /* Enable clock in active mode */
  ti_lib_rom_prcm_peripheral_run_enable(PRCM_PERIPH_SSI0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* SPI configuration */
  ti_lib_ssi_int_disable(SSI0_BASE, SSI_RXOR | SSI_RXFF | SSI_RXTO | SSI_TXFF);
  ti_lib_ssi_int_clear(SSI0_BASE, SSI_RXOR | SSI_RXTO);
  ti_lib_rom_ssi_config_set_exp_clk(SSI0_BASE, ti_lib_sys_ctrl_clock_get(),
                                    SSI_FRF_MOTO_MODE_0,
                                    SSI_MODE_MASTER, bit_rate, 8);
  ti_lib_rom_ioc_pin_type_ssi_master(SSI0_BASE, BOARD_IOID_SPI_MISO,
                                     BOARD_IOID_SPI_MOSI, IOID_UNUSED, clk_pin);
  ti_lib_ssi_enable(SSI0_BASE);

  /* Get rid of residual data from SSI port */
  while(ti_lib_ssi_data_get_non_blocking(SSI0_BASE, &buf));
}
/*---------------------------------------------------------------------------*/
void board_spi_close()
{
  /* Power down SSI0 */
  ti_lib_rom_prcm_peripheral_run_disable(PRCM_PERIPH_SSI0);
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Restore pins to a low-consumption state */
  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_MISO);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_SPI_MISO, IOC_IOPULL_DOWN);

  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_MOSI);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_SPI_MOSI, IOC_IOPULL_DOWN);

  ti_lib_ioc_pin_type_gpio_input(BOARD_IOID_SPI_CLK_FLASH);
  ti_lib_ioc_io_port_pull_set(BOARD_IOID_SPI_CLK_FLASH, IOC_IOPULL_DOWN);
}
/*---------------------------------------------------------------------------*/
/** @} */
