/*

 * \addtogroup sensortag-cc26xx-tmp-sensor
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26xx TI TMP007 infrared thermophile sensor
 */
/*---------------------------------------------------------------------------*/
#include "tmp-007-sensor.h"
#include "board-i2c.h"
#include "sensor-common.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ti-lib.h>
#include "board.h"

/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
/* Slave address */
#define SENSOR_I2C_ADDRESS              0x44
/*---------------------------------------------------------------------------*/
/* TMP007 register addresses */
#define TMP007_REG_ADDR_VOLTAGE         0x00
#define TMP007_REG_ADDR_LOCAL_TEMP      0x01
#define TMP007_REG_ADDR_CONFIG          0x02
#define TMP007_REG_ADDR_OBJ_TEMP        0x03
#define TMP007_REG_ADDR_STATUS          0x04
#define TMP007_REG_PROD_ID              0x1F
/*---------------------------------------------------------------------------*/
/* TMP007 register values */
#define TMP007_VAL_CONFIG_ON            0x1000  /* Sensor on state */
#define TMP007_VAL_CONFIG_OFF           0x0000  /* Sensor off state */
#define TMP007_VAL_CONFIG_RESET         0x8000
#define TMP007_VAL_PROD_ID              0x0078  /* Product ID */
/*---------------------------------------------------------------------------*/
/* Conversion ready (status register) bit values */
#define CONV_RDY_BIT                    0x4000
/*---------------------------------------------------------------------------*/
/* Register length */
#define REGISTER_LENGTH                 2
/*---------------------------------------------------------------------------*/
/* Sensor data size */
#define DATA_SIZE                       4
/*---------------------------------------------------------------------------*/
/* Byte swap of 16-bit register value */
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define SWAP(v) ((LO_UINT16(v) << 8) | HI_UINT16(v))
/*---------------------------------------------------------------------------*/
#define SELECT() board_i2c_select(BOARD_I2C_INTERFACE_0, SENSOR_I2C_ADDRESS)
/*---------------------------------------------------------------------------*/
static uint8_t buf[DATA_SIZE];
static uint16_t val;
/*---------------------------------------------------------------------------*/
#define SENSOR_STATUS_DISABLED     0
#define SENSOR_STATUS_INITIALISED  1
#define SENSOR_STATUS_NOT_READY    2
#define SENSOR_STATUS_READY        3

/* some constants for the configure API */
#define SENSORS_HW_INIT 128 /* internal - used only for initialization */
#define SENSORS_ACTIVE 129 /* ACTIVE => 0 -> turn off, 1 -> turn on */
#define SENSORS_READY 130 /* read only */


#define CC26XX_SENSOR_READING_ERROR        0x80000000

/*---------------------------------------------------------------------------*/
/* Wait SENSOR_STARTUP_DELAY clock ticks for the sensor to be ready - 275ms */
#define SENSOR_STARTUP_DELAY 36

/*---------------------------------------------------------------------------*/
/* Configuation temp-Sensors */
uint16_t g_temp_calibration = 12320; 	  // in mC�

// 29.230 + 28.410 = 57.640				 // in C�
// 2 * 16.500 = 33.0000
// Diff = 57.640 - 33.000 = 24.640		 // for 2 sensors


/*---------------------------------------------------------------------------*/
/* Latched values */
static int obj_temp_latched;
static int amb_temp_latched;

/*---------------------------------------------------------------------------*/
/**
 * \brief Turn the sensor on/off
 */
bool enable_tmp_007(bool enable)
{
  bool success;

  SELECT();

  if(enable) {
    val = TMP007_VAL_CONFIG_ON;
  } else {
    val = TMP007_VAL_CONFIG_OFF;
  }
  val = SWAP(val);

  success = sensor_common_write_reg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val,
                                   REGISTER_LENGTH);

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Read the sensor value registers
 * \param raw_temp Temperature in 16 bit format
 * \param raw_obj_temp object temperature in 16 bit format
 * \return TRUE if valid data could be retrieved
 */
bool read_data_tmp_007(uint16_t *raw_temp, uint16_t *raw_obj_temp)
{
  bool success;

  SELECT();

  success = sensor_common_read_reg(TMP007_REG_ADDR_STATUS, (uint8_t *)&val,
                                   REGISTER_LENGTH);

  if(success) {
    val = SWAP(val);
    success = val & CONV_RDY_BIT;
  }

  if(success) {
    success = sensor_common_read_reg(TMP007_REG_ADDR_LOCAL_TEMP, &buf[0],
                                     REGISTER_LENGTH);
    if(success) {
      success = sensor_common_read_reg(TMP007_REG_ADDR_OBJ_TEMP, &buf[2],
                                       REGISTER_LENGTH);
    }
  }

  if(!success) {
    sensor_common_set_error_data(buf, 4);
  }

  /* Swap byte order */
  *raw_temp = buf[0] << 8 | buf[1];
  *raw_obj_temp = buf[2] << 8 | buf[3];

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert raw data to values in degrees C
 * \param raw_temp raw ambient temperature from sensor
 * \param raw_obj_temp raw object temperature from sensor
 * \param obj converted object temperature
 * \param amb converted ambient temperature
 */
void convert_tmp_007(uint16_t raw_temp, uint16_t raw_obj_temp, float *obj, float *amb)
{
  const float SCALE_LSB = 0.03125;
  float t;
  int it;

  it = (int)((raw_obj_temp) >> 2);
  t = ((float)(it)) * SCALE_LSB;
  *obj = t;

  it = (int)((raw_temp) >> 2);
  t = (float)it;
  *amb = t * SCALE_LSB;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param type TMP_007_SENSOR_TYPE_OBJECT or TMP_007_SENSOR_TYPE_AMBIENT
 * \return Object or Ambient temperature in milli degrees C
 */
int value_tmp_007(int type)
{
  int rv;
  uint16_t raw_temp;
  uint16_t raw_obj_temp;
  float obj_temp;
  float amb_temp;

  if((type & TMP_007_SENSOR_TYPE_ALL) == 0) {
    PRINTF("Invalid type\n");
    return CC26XX_SENSOR_READING_ERROR;
  }

  rv = CC26XX_SENSOR_READING_ERROR;

  if(type == TMP_007_SENSOR_TYPE_ALL | TMP_007_SENSOR_TYPE_AMBIENT) {
    rv = read_data_tmp_007(&raw_temp, &raw_obj_temp);      // read()

    if(rv == 0) {
      return CC26XX_SENSOR_READING_ERROR;
    }

    convert_tmp_007(raw_temp, raw_obj_temp, &obj_temp, &amb_temp);  //convert
    PRINTF("TMP: %04X %04X       o=%d a=%d\n", raw_temp, raw_obj_temp,
           (int)(obj_temp * 1000), (int)(amb_temp * 1000));

    obj_temp_latched = (int)(obj_temp * 1000);
    amb_temp_latched = (int)(amb_temp * 1000);
    rv = 1;
  } else if(type == TMP_007_SENSOR_TYPE_OBJECT) {
    rv = obj_temp_latched;
  } else if(type == TMP_007_SENSOR_TYPE_AMBIENT) {
    rv = amb_temp_latched;
  }

  // add calibration to current sensor value
  rv = amb_temp_latched - g_temp_calibration;
  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the TMP007 sensor.
 *
 * \param type Activate, enable or disable the sensor. See below
 * \param enable
 *
 * When type == SENSORS_HW_INIT we turn on the hardware
 * When type == SENSORS_ACTIVE and enable==1 we enable the sensor
 * When type == SENSORS_ACTIVE and enable==0 we disable the sensor
 */
int configure_tmp_007(int enable)
{
	IOCPinTypeGpioInput(BOARD_IOID_TMP_RDY);
	ti_lib_ioc_io_port_pull_set(BOARD_IOID_TMP_RDY, IOC_IOPULL_UP);
	ti_lib_ioc_io_hyst_set(BOARD_IOID_TMP_RDY, IOC_HYST_ENABLE);

	enable_tmp_007(enable);
}


