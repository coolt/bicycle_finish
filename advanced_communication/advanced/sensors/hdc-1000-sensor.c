/*

 * \addtogroup sensortag-cc26xx-hdc-sensor
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26xx HDC sensor
 */
/*---------------------------------------------------------------------------*/

#include "hdc-1000-sensor.h"
#include "sensor-common.h"
#include "board-i2c.h"
#include "board.h"

#include "ti-lib.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Sensor I2C address */
#define SENSOR_I2C_ADDRESS         0x43

/* Registers */
#define HDC1000_REG_TEMP           0x00 /* Temperature */
#define HDC1000_REG_HUM            0x01 /* Humidity */
#define HDC1000_REG_CONFIG         0x02 /* Configuration */
#define HDC1000_REG_SERID_H        0xFB /* Serial ID high */
#define HDC1000_REG_SERID_M        0xFC /* Serial ID middle */
#define HDC1000_REG_SERID_L        0xFD /* Serial ID low */
#define HDC1000_REG_MANF_ID        0xFE /* Manufacturer ID */
#define HDC1000_REG_DEV_ID         0xFF /* Device ID */

/* Fixed values */
#define HDC1000_VAL_MANF_ID        0x5449
#define HDC1000_VAL_DEV_ID         0x1000
#define HDC1000_VAL_CONFIG         0x1000 /* 14 bit, acquired in sequence */

/* Platform-specific define to signify sensor reading failure */
#define CC26XX_SENSOR_READING_ERROR        0x80000000

/* Sensor selection/deselection */
#define SENSOR_SELECT()     board_i2c_select(BOARD_I2C_INTERFACE_0, SENSOR_I2C_ADDRESS)
#define SENSOR_DESELECT()   board_i2c_deselect()
/*---------------------------------------------------------------------------*/
/* Byte swap of 16-bit register value */
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define SWAP(v) ((LO_UINT16(v) << 8) | HI_UINT16(v))
/*---------------------------------------------------------------------------*/
/* Raw data as returned from the sensor (Big Endian) */
typedef struct sensor_data {
  uint16_t temp;
  uint16_t hum;
} sensor_data_t;

/* Raw data, little endian */
static uint16_t raw_temp;
static uint16_t raw_hum;
/*---------------------------------------------------------------------------*/
static bool success;
static sensor_data_t data;

/*---------------------------------------------------------------------------*/
/**
 * \brief       Initialise the humidity sensor driver
 * \return      True if I2C operation successful
 */
bool init_hdc_1000(void)
{
  uint16_t val;

  SENSOR_SELECT();

  /* Enable reading data in one operation */
  val = SWAP(HDC1000_VAL_CONFIG);
  success = sensor_common_write_reg(HDC1000_REG_CONFIG, (uint8_t *)&val, 2);

  SENSOR_DESELECT();

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief       Start measurement
 */
void start_hdc_1000(void)
{
  if(success) {
    SENSOR_SELECT();

    success = board_i2c_write_single(HDC1000_REG_TEMP);
    SENSOR_DESELECT();
  }
}
/*---------------------------------------------------------------------------*/
/**
 * \brief       Take readings from the sensor
 * \return      true of I2C operations successful
 */
bool read_data_hdc_1000()
{
  bool valid;

  if(success) {
    SENSOR_SELECT();

    success = board_i2c_read((uint8_t *)&data, sizeof(data));
    SENSOR_DESELECT();

    /* Store temperature */
    raw_temp = SWAP(data.temp);

    /* Store humidity */
    raw_hum = SWAP(data.hum);
  }

  valid = success;
  success = true;

  return valid;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief       Convert raw data to temperature and humidity
 * \param       temp - converted temperature
 * \param       hum - converted humidity
 */
void convert_hdc_1000(float *temp, float *hum)
{
  /* Convert temperature to degrees C */
  *temp = ((double)(int16_t)raw_temp / 65536) * 165 - 40;

  /* Convert relative humidity to a %RH value */
  *hum = ((double)raw_hum / 65536) * 100;
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param type HDC_1000_SENSOR_TYPE_TEMP or HDC_1000_SENSOR_TYPE_HUMIDITY
 * \return Temperature (centi degrees C) or Humidity (centi %RH)
 */
int value_hdc_1000(int type)
{
  int rv;
  float temp;
  float hum;

  if((type != HDC_1000_SENSOR_TYPE_TEMP) &&
     type != HDC_1000_SENSOR_TYPE_HUMIDITY) {
    PRINTF("Invalid type\n");
    return CC26XX_SENSOR_READING_ERROR;
  } else {
	convert_hdc_1000(&temp, &hum);				// from register row format to degree
    PRINTF("HDC: %04X %04X       t=%d h=%d\n", raw_temp, raw_hum,
           (int)(temp * 100), (int)(hum * 100));

    if(type == HDC_1000_SENSOR_TYPE_TEMP) {
      rv = (int)(temp * 100);
    } else if(type == HDC_1000_SENSOR_TYPE_HUMIDITY) {
      rv = (int)(hum * 100);
    }
  }
  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the HDC1000 sensor.
 *
 * \param type Activate, enable or disable the sensor. See below
 * \param enable
 *
 * When type == SENSORS_HW_INIT we turn on the hardware
 * When type == SENSORS_ACTIVE and enable==1 we enable the sensor
 * When type == SENSORS_ACTIVE and enable==0 we disable the sensor
 */
void configure_hdc_1000(void)
{
	memset(&data, 0, sizeof(data));

	init_hdc_1000();
}

/*---------------------------------------------------------------------------*/
/** @} */
