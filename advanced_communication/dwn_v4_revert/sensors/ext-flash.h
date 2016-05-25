/*
  * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-ext-flash SensorTag 2.0 External Flash
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26xx External Flash Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef EXT_FLASH_H_
#define EXT_FLASH_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize storage driver.
 * \return True when successful.
 */
bool ext_flash_open(void);

/**
 * \brief Close the storage driver
 *
 * This call will put the device in its lower power mode (power down).
 */
void ext_flash_close(void);

/**
 * \brief Read storage content
 * \param offset Address to read from
 * \param length Number of bytes to read
 * \param buf Buffer where to store the read bytes
 * \return True when successful.
 *
 * buf must be allocated by the caller
 */
bool ext_flash_read(size_t offset, size_t length, uint8_t *buf);

/**
 * \brief Erase storage sectors corresponding to the range.
 * \param offset Address to start erasing
 * \param length Number of bytes to erase
 * \return True when successful.
 *
 * The erase operation will be sector-wise, therefore a call to this function
 * will generally start the erase procedure at an address lower than offset
 */
bool ext_flash_erase(size_t offset, size_t length);

/**
 * \brief Write to storage sectors.
 * \param offset Address to write to
 * \param length Number of bytes to write
 * \param buf Buffer holding the bytes to be written
 *
 * \return True when successful.
 */
bool ext_flash_write(size_t offset, size_t length, const uint8_t *buf);

/**
 * \brief Test the flash (power on self-test)
 * \return True when successful.
 */
bool ext_flash_test(void);

/**
 * \brief Initialise the external flash
 *
 * This function will explicitly put the part in its lowest power mode
 * (power-down).
 *
 * In order to perform any operation, the caller must first wake the device
 * up by calling ext_flash_open()
 */
void ext_flash_init(void);
/*---------------------------------------------------------------------------*/
#endif /* EXT_FLASH_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
