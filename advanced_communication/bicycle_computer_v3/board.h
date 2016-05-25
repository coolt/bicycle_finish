/*
 * Definition for the bicycle computer in the end of file
 * The names are based on the netnames of the pcb
 */


#ifndef BOARD_H_
#define BOARD_H_

#include "cc26xxware_2_22_00_16101/driverLib/ioc.h"


// LED
/* Some files include leds.h before us, so we need to get rid of defaults in
 * leds.h before we provide correct definitions */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_RED
#undef LEDS_CONF_ALL

#define LEDS_RED       1
#define LEDS_GREEN     2
#define LEDS_YELLOW    LEDS_GREEN
#define LEDS_ORANGE    LEDS_RED

#define LEDS_CONF_ALL  3
#define PLATFORM_HAS_LEDS        1

#define BOARD_IOID_LED_1          IOID_10
#define BOARD_IOID_LED_2          IOID_15
#define BOARD_LED_1               (1 << BOARD_IOID_LED_1)
#define BOARD_LED_2               (1 << BOARD_IOID_LED_2)
#define BOARD_LED_ALL             (BOARD_LED_1 | BOARD_LED_2)


// UART
#define BOARD_IOID_DP4_UARTRX     IOID_28
#define BOARD_IOID_DP5_UARTTX     IOID_29

#if BOARD_CONF_DEBUGGER_DEVPACK
#define BOARD_IOID_UART_RX        BOARD_IOID_DP4_UARTRX
#define BOARD_IOID_UART_TX        BOARD_IOID_DP5_UARTTX
#else
#define BOARD_IOID_UART_RX        IOID_17
#define BOARD_IOID_UART_TX        IOID_16
#endif

#define BOARD_IOID_UART_CTS       IOID_UNUSED
#define BOARD_IOID_UART_RTS       IOID_UNUSED
#define BOARD_UART_RX             (1 << BOARD_IOID_UART_RX)
#define BOARD_UART_TX             (1 << BOARD_IOID_UART_TX)
#define BOARD_UART_CTS            (1 << BOARD_IOID_UART_CTS)
#define BOARD_UART_RTS            (1 << BOARD_IOID_UART_RTS)

// Button
#define BOARD_IOID_KEY_LEFT       IOID_0
#define BOARD_IOID_KEY_RIGHT      IOID_4
#define BOARD_KEY_LEFT            (1 << BOARD_IOID_KEY_LEFT)
#define BOARD_KEY_RIGHT           (1 << BOARD_IOID_KEY_RIGHT)

// SPI
#define BOARD_IOID_SPI_MOSI       IOID_19
#define BOARD_IOID_SPI_MISO       IOID_18

// Buzzer
#define BOARD_IOID_BUZZER         IOID_21 /**< Buzzer Pin */

// Reed Relay
#define BOARD_IOID_REED_RELAY     IOID_3

// Flash
#define BOARD_IOID_FLASH_CS       IOID_14
#define BOARD_FLASH_CS            (1 << BOARD_IOID_FLASH_CS)
#define BOARD_IOID_SPI_CLK_FLASH  IOID_17

// I2C
#define BOARD_IOID_SDA            IOID_5 /**< Interface 0 SDA: All sensors bar MPU */
#define BOARD_IOID_SCL            IOID_6 /**< Interface 0 SCL: All sensors bar MPU */
#define BOARD_IOID_SDA_HP         IOID_8 /**< Interface 1 SDA: MPU */
#define BOARD_IOID_SCL_HP         IOID_9 /**< Interface 1 SCL: MPU */

// MPU
#define BOARD_IOID_MPU_INT        IOID_7
#define BOARD_IOID_MPU_POWER      IOID_12
#define BOARD_MPU_INT             (1 << BOARD_IOID_MPU_INT)
#define BOARD_MPU_POWER           (1 << BOARD_IOID_MPU_POWER)

// Dev Pack (as LCD, ..)
#define BOARD_IOID_AUDIOFS_TDO        IOID_16
#define BOARD_IOID_DEVPACK_CS         IOID_20
#define BOARD_IOID_DEVPK_LCD_EXTCOMIN IOID_22
#define BOARD_IOID_AUDIODO            IOID_22
#define BOARD_IOID_DP2                IOID_23
#define BOARD_IOID_DP1                IOID_24
#define BOARD_IOID_DP0                IOID_25
#define BOARD_IOID_DP3                IOID_27
#define BOARD_IOID_DEVPK_ID           IOID_30
#define BOARD_DEVPACK_CS              (1 << BOARD_IOID_DEVPACK_CS)

// tmp Sensor
#define BOARD_IOID_TMP_RDY          IOID_1

// digital Mic
#define BOARD_IOID_MIC_POWER        IOID_13
#define BOARD_IOID_AUDIO_DI         IOID_2
#define BOARD_IOID_AUDIO_CLK        IOID_11



// device string
#define BOARD_STRING "TI CC2650 SensorTag"

#endif /* BOARD_H_ */

