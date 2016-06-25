/*
 * spi.h
 *
 *  Created on: 04.05.2016
 *      Author: katrin
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include <config.h>
#include <board.h>

// global variables
uint8_t spiBuffer[SPI_BUFFER_LENGTH];
uint8_t em8500_config_data[EM_CONFIG_BUFFER_LENGTH ];


// em8500 config data
// ----------------------
// structure: adress (1 byte), value (1 byte)

#define ADDRESS_T_HRV_PERIOD 		0x00
#define ADDRESS_T_HRV_MEAS			0x01

#define VALUE_T_HRV_PERIOD			0x44 // just for test


// * Functions
// ------------
uint8_t EM8500read(uint8_t address);		// can read any address
uint8_t readStatusRegisterEM8500(void);

void configureEM8500(void);




// imported values from read out em8500 config
/*
em8500_config_data[0x40] = 0x06;					// check: spi-read 32 bit, here buffer of 8 bits
em8500_config_data[0x41] = 0x06;
em8500_config_data[0x42] = 0x02;
em8500_config_data[0x43] = 0x05;
em8500_config_data[0x44] = 0x00;
em8500_config_data[0x45] = 0x04;
em8500_config_data[0x46] = 0x00;
em8500_config_data[0x47] = 0x2a;
em8500_config_data[0x48] = 0x29;
em8500_config_data[0x49] =0x1e
em8500_config_data[0x4a] =0x1e
em8500_config_data[0x4b=0x1d
em8500_config_data[0x4c=0x25
em8500_config_data[0x4d=0x21
em8500_config_data[0x4e=0xcf
em8500_config_data[0x4f=0x7e
em8500_config_data[0x50=0x15
em8500_config_data[0x51=0x01
em8500_config_data[0x52=0x06
em8500_config_data[0x53=0x65
em8500_config_data[0x54=0x99
em8500_config_data[0x55=0x3a
em8500_config_data[0x56=0x00
em8500_config_data[0x57=0x67
em8500_config_data[0x58=0x1b

*/





#endif /* SPI_H_ */
