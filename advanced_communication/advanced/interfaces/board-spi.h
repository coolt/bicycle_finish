/*
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-spi SensorTag 2.0 SPI functions
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26xx SPI Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_SPI_H_
#define BOARD_SPI_H_
/*---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


bool accessible(void);

/*---------------------------------------------------------------------------*/
/**
 * \brief Initialize the SPI interface
 * \param bit_rate The bit rate to use
 * \param clk_pin The IOID for the clock pin. This can be IOID_0 etc
 * \return none
 *
 * This function will make sure the peripheral is powered, clocked and
 * initialised. A chain of calls to board_spi_read(), board_spi_write() and
 * board_spi_flush() must be preceded by a call to this function. It is
 * recommended to call board_spi_close() after such chain of calls.
 */
void board_spi_open(uint32_t bit_rate, uint32_t clk_pin);

/**
 * \brief Close the SPI interface
 * \return True when successful.
 *
 * This function will stop clocks to the SSI module and will set MISO, MOSI
 * and CLK to a low leakage state. It is recommended to call this function
 * after a chain of calls to board_spi_read() and board_spi_write()
 */
void board_spi_close(void);

/**
 * \brief Clear data from the SPI interface
 * \return none
 */
void board_spi_flush(void);

/**
 * \brief Read from an SPI device
 * \param buf The buffer to store data
 * \param length The number of bytes to read
 * \return True when successful.
 *
 * Calls to this function must be preceded by a call to board_spi_open(). It is
 * recommended to call board_spi_close() at the end of an operation.
 */
bool board_spi_read(uint8_t *buf, size_t length);

/**
 * \brief Write to an SPI device
 * \param buf The buffer with the data to write
 * \param length The number of bytes to write
 * \return True when successful.
 *
 * Calls to this function must be preceded by a call to board_spi_open(). It is
 * recommended to call board_spi_close() at the end of an operation.
 */
bool board_spi_write(const uint8_t *buf, size_t length);
/*---------------------------------------------------------------------------*/
#endif /* BOARD_SPI_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
