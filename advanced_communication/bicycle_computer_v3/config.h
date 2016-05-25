// Configuration application
#include <inc/hw_types.h>

//* @note: first definitions in board.h
#define REED_SWITCH					BOARD_IOID_DP0   // IOID_25
#define VCC_LTS						BOARD_IOID_DP1   // IOID_24 (analog)
#define VCC_STS						BOARD_IOID_DP2   // IOID_23
#define VREG						BOARD_IOID_DP3   // IOID_27
#define BAT_LOW						BOARD_IOID_DP4_UARTRX // IOID_28
#define HRV_LOW						BOARD_IOID_DP5_UARTTX // IOID_29
#define WAKE_UP						BOARD_IOID_AUDIODO    // IOID_22 (from sensortag to em-board)
#define EM_CS						BOARD_IOID_DEVPACK_CS // IOID_20
#define MCU_MOSI					BOARD_IOID_SPI_MOSI   // IOID_19
#define MCU_MISO					BOARD_IOID_SPI_MISO   // IOID_18
#define MCU_SCLK					BOARD_IOID_SPI_CLK_FLASH // IOID_17
#define BUTTON						BOARD_IOID_KEY_RIGHT     // IOID_4



// RTC wakeup interval
// --------------------
// Struktur 0x0000'0000		= obere 2 Byte [s], untere 2 Byte [ms]
// Bsp.     0x0005'4000     = 5 s + 0.5 ms

#define WAKE_INTERVAL_LOW_ENERGY 	0x000A0000   	// 10 s
#define WAKE_INTERVAL_MIDDLE_ENERGY 0x00030000 		//  3 s
#define WAKE_INTERVAL_HIGH_ENERGY 	0x00010000		//  1 s

extern long g_current_wake_up_time/* = WAKE_INTERVAL_HIGH_ENERGY*/;		// = max. = 256 s = 4.5 h

// Energy management
// -----------------
#define LOW_ENERGY 					0x01
#define MIDDLE_ENERGY 				0x02
#define HIGH_ENERGY 				0x04

extern long g_current_energy_state;

//* Speed measurement
// -----------------
extern uint32_t g_timestamp1, g_timestamp2;
extern bool g_measurement_done;


//* Radio data
// ------------
// Length of Data-Block (inclusive length, type, uuid)
#define ADVLEN 16

extern char payload[ADVLEN]; 						// data buffer
extern volatile bool rfBootDone;					// communication flag
extern volatile bool rfSetupDone;					// communication flag
extern volatile bool rfAdvertisingDone;				// communication flag

// button
extern bool g_button_pressed;

// sensor
// -------
extern bool g_pressure_set;
extern uint16_t g_pressure;

