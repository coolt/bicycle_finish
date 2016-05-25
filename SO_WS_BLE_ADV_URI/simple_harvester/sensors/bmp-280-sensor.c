/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-bmp-sensor
 * @{
 *
 * \file
 *  Driver for the Sensortag-CC26XX BMP280 Altimeter / Pressure Sensor
 */
/*---------------------------------------------------------------------------*/
#include "bmp-280-sensor.h"
#include "sensor-common.h"
#include "board-i2c.h"
#include "ti-lib.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define BMP280_I2C_ADDRESS                  0x77
/*---------------------------------------------------------------------------*/
/* Registers */
#define ADDR_CALIB                          0x88
#define ADDR_PROD_ID                        0xD0
#define ADDR_RESET                          0xE0
#define ADDR_STATUS                         0xF3
#define ADDR_CTRL_MEAS                      0xF4
#define ADDR_CONFIG                         0xF5
#define ADDR_PRESS_MSB                      0xF7
#define ADDR_PRESS_LSB                      0xF8
#define ADDR_PRESS_XLSB                     0xF9
#define ADDR_TEMP_MSB                       0xFA
#define ADDR_TEMP_LSB                       0xFB
#define ADDR_TEMP_XLSB                      0xFC
/*---------------------------------------------------------------------------*/
/* Reset values */
#define VAL_PROD_ID                         0x58
#define VAL_RESET                           0x00
#define VAL_STATUS                          0x00
#define VAL_CTRL_MEAS                       0x00
#define VAL_CONFIG                          0x00
#define VAL_PRESS_MSB                       0x80
#define VAL_PRESS_LSB                       0x00
#define VAL_TEMP_MSB                        0x80
#define VAL_TEMP_LSB                        0x00
/*---------------------------------------------------------------------------*/
/* Test values */
#define VAL_RESET_EXECUTE                   0xB6
#define VAL_CTRL_MEAS_TEST                  0x55
/*---------------------------------------------------------------------------*/
/* Misc. */
#define MEAS_DATA_SIZE                      6
#define CALIB_DATA_SIZE                     24
/*---------------------------------------------------------------------------*/
#define RES_OFF                             0
#define RES_ULTRA_LOW_POWER                 1
#define RES_LOW_POWER                       2
#define RES_STANDARD                        3
#define RES_HIGH                            5
#define RES_ULTRA_HIGH                      6
/*---------------------------------------------------------------------------*/
/* Bit fields in CTRL_MEAS register */
#define PM_OFF                              0
#define PM_FORCED                           1
#define PM_NORMAL                           3
/*---------------------------------------------------------------------------*/
#define OSRST(v)                            ((v) << 5)
#define OSRSP(v)                            ((v) << 2)
/*---------------------------------------------------------------------------*/
typedef struct bmp_280_calibration {
  uint16_t dig_t1;
  int16_t dig_t2;
  int16_t dig_t3;
  uint16_t dig_p1;
  int16_t dig_p2;
  int16_t dig_p3;
  int16_t dig_p4;
  int16_t dig_p5;
  int16_t dig_p6;
  int16_t dig_p7;
  int16_t dig_p8;
  int16_t dig_p9;
  int32_t t_fine;
} bmp_280_calibration_t;
/*---------------------------------------------------------------------------*/
static uint8_t calibration_data[CALIB_DATA_SIZE];
/*---------------------------------------------------------------------------*/
#define SENSOR_STATUS_DISABLED     0
#define SENSOR_STATUS_INITIALISED  1
#define SENSOR_STATUS_NOT_READY    2
#define SENSOR_STATUS_READY        3

static int enabled = SENSOR_STATUS_DISABLED;
/*---------------------------------------------------------------------------*/
/* A buffer for the raw reading from the sensor */
#define SENSOR_DATA_BUF_SIZE   6

static uint8_t sensor_value[SENSOR_DATA_BUF_SIZE];
/*---------------------------------------------------------------------------*/
/* Wait SENSOR_STARTUP_DELAY clock ticks for the sensor to be ready - ~80ms */
#define SENSOR_STARTUP_DELAY 3

/*---------------------------------------------------------------------------*/
/* Platform-specific define to signify sensor reading failure */
#define CC26XX_SENSOR_READING_ERROR        0x80000000

/*---------------------------------------------------------------------------*/
void select_bmp_280(void)
{
  /* Set up I2C */
  board_i2c_select(BOARD_I2C_INTERFACE_0, BMP280_I2C_ADDRESS);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Initalise the sensor
 */
void init_bmp_280(void)
{
  uint8_t val;

  select_bmp_280();

  /* Read and store calibration data */
  sensor_common_read_reg(ADDR_CALIB, calibration_data, CALIB_DATA_SIZE);

  /* Reset the sensor */
  val = VAL_RESET_EXECUTE;
  sensor_common_write_reg(ADDR_RESET, &val, sizeof(val));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Enable/disable measurements
 * \param enable 0: disable, enable otherwise
 *
 * @return      none
 */
void enable_bmp_280(bool enable)
{
  uint8_t val;

  select_bmp_280();

  if(enable) {
    /* Enable forced mode */
    val = PM_FORCED | OSRSP(1) | OSRST(1);
  } else {
    val = PM_OFF;
  }
  sensor_common_write_reg(ADDR_CTRL_MEAS, &val, sizeof(val));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Read temperature and pressure data
 * \param data Pointer to a buffer where temperature and pressure will be
 *             written (6 bytes)
 * \return True if valid data could be retrieved
 */
bool read_data_bmp_280(uint8_t *data)
{
  bool success;

  select_bmp_280();

  success = sensor_common_read_reg(ADDR_PRESS_MSB, data, MEAS_DATA_SIZE);
  if(!success) {
    sensor_common_set_error_data(data, MEAS_DATA_SIZE);
  }

  return success;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Convert raw data to values in degrees C (temp) and Pascal (pressure)
 * \param data Pointer to a buffer that holds raw sensor data
 * \param temp Pointer to a variable where the converted temperature will be
 *             written
 * \param press Pointer to a variable where the converted pressure will be
 *              written
 */
void convert_bmp_280(uint8_t *data, int32_t *temp, uint32_t *press)
{
  int32_t utemp, upress;
  bmp_280_calibration_t *p = (bmp_280_calibration_t *)calibration_data;
  int32_t v_x1_u32r;
  int32_t v_x2_u32r;
  int32_t temperature;
  uint32_t pressure;

  /* Pressure */
  upress = (int32_t)((((uint32_t)(data[0])) << 12)
                     | (((uint32_t)(data[1])) << 4) | ((uint32_t)data[2] >> 4));

  /* Temperature */
  utemp = (int32_t)((((uint32_t)(data[3])) << 12) | (((uint32_t)(data[4])) << 4)
                    | ((uint32_t)data[5] >> 4));

  /* Compensate temperature */
  v_x1_u32r = ((((utemp >> 3) - ((int32_t)p->dig_t1 << 1)))
               * ((int32_t)p->dig_t2)) >> 11;
  v_x2_u32r = (((((utemp >> 4) - ((int32_t)p->dig_t1))
                 * ((utemp >> 4) - ((int32_t)p->dig_t1))) >> 12)
               * ((int32_t)p->dig_t3))
    >> 14;
  p->t_fine = v_x1_u32r + v_x2_u32r;
  temperature = (p->t_fine * 5 + 128) >> 8;
  *temp = temperature;

  /* Compensate pressure */
  v_x1_u32r = (((int32_t)p->t_fine) >> 1) - (int32_t)64000;
  v_x2_u32r = (((v_x1_u32r >> 2) * (v_x1_u32r >> 2)) >> 11)
    * ((int32_t)p->dig_p6);
  v_x2_u32r = v_x2_u32r + ((v_x1_u32r * ((int32_t)p->dig_p5)) << 1);
  v_x2_u32r = (v_x2_u32r >> 2) + (((int32_t)p->dig_p4) << 16);
  v_x1_u32r =
    (((p->dig_p3 * (((v_x1_u32r >> 2) * (v_x1_u32r >> 2)) >> 13)) >> 3)
     + ((((int32_t)p->dig_p2) * v_x1_u32r) >> 1)) >> 18;
  v_x1_u32r = ((((32768 + v_x1_u32r)) * ((int32_t)p->dig_p1)) >> 15);

  if(v_x1_u32r == 0) {
    return; /* Avoid exception caused by division by zero */
  }

  pressure = (((uint32_t)(((int32_t)1048576) - upress) - (v_x2_u32r >> 12)))
    * 3125;
  if(pressure < 0x80000000) {
    pressure = (pressure << 1) / ((uint32_t)v_x1_u32r);
  } else {
    pressure = (pressure / (uint32_t)v_x1_u32r) * 2;
  }

  v_x1_u32r = (((int32_t)p->dig_p9)
               * ((int32_t)(((pressure >> 3) * (pressure >> 3)) >> 13))) >> 12;
  v_x2_u32r = (((int32_t)(pressure >> 2)) * ((int32_t)p->dig_p8)) >> 13;
  pressure = (uint32_t)((int32_t)pressure
                        + ((v_x1_u32r + v_x2_u32r + p->dig_p7) >> 4));

  *press = pressure;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param BMP_280_SENSOR_TYPE_TEMP or BMP_280_SENSOR_TYPE_PRESS
 * \return Temperature (centi degrees C) or Pressure (Pascal).
 */
int value_bmp_280(int type)
{
  int rv;
  int32_t temp = 0;
  uint32_t pres = 0;

  if(enabled != SENSOR_STATUS_READY) {
    PRINTF("Sensor disabled or starting up (%d)\n", enabled);
    return 0x03;
  }

  if((type != BMP_280_SENSOR_TYPE_TEMP) && type != BMP_280_SENSOR_TYPE_PRESS) {
    PRINTF("Invalid type\n");
    return CC26XX_SENSOR_READING_ERROR;
  } else {
    memset(sensor_value, 0, SENSOR_DATA_BUF_SIZE);

    rv = read_data_bmp_280(sensor_value);

    if(rv == 0) {
      return CC26XX_SENSOR_READING_ERROR;
    }

    PRINTF("val: %02x%02x%02x %02x%02x%02x\n",
           sensor_value[0], sensor_value[1], sensor_value[2],
           sensor_value[3], sensor_value[4], sensor_value[5]);

    convert_bmp_280(sensor_value, &temp, &pres);

    if(type == BMP_280_SENSOR_TYPE_TEMP) {
      rv = (int)temp;
    } else if(type == BMP_280_SENSOR_TYPE_PRESS) {
      rv = (int)pres;
    }
  }
  return rv;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the BMP280 sensor.
 *
 * \param type Activate, enable or disable the sensor. See below
 * \param enable
 *
 * When type == SENSORS_HW_INIT we turn on the hardware
 * When type == SENSORS_ACTIVE and enable==1 we enable the sensor
 * When type == SENSORS_ACTIVE and enable==0 we disable the sensor
 */
int configure_bmp_280(int enable)
{
	init_bmp_280();
	enable_bmp_280(enable);

	return enable;
}

/** @} */
