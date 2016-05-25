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
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-bmp-sensor SensorTag 2.0 Pressure Sensor
 *
 * Due to the time required for the sensor to startup, this driver is meant to
 * be used in an asynchronous fashion. The caller must first activate the
 * sensor by calling SENSORS_ACTIVATE(). This will trigger the sensor's startup
 * sequence, but the call will not wait for it to complete so that the CPU can
 * perform other tasks or drop to a low power mode.
 *
 * Once the sensor is stable, the driver will generate a sensors_changed event.
 *
 * We take readings in "Forced" mode. In this mode, the BMP will take a single
 * measurement and it will then automatically go to sleep.
 *
 * SENSORS_ACTIVATE must be called again to trigger a new reading cycle
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26xx BMP280 Altimeter / Pressure Sensor
 */
/*---------------------------------------------------------------------------*/
#ifndef BMP_280_SENSOR_H_
#define BMP_280_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define BMP_280_SENSOR_TYPE_TEMP    1
#define BMP_280_SENSOR_TYPE_PRESS   2
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor bmp_280_sensor;


/*---------------------------------------------------------------------------*/
void select_bmp_280(void);

/*---------------------------------------------------------------------------*/
/**
 * \brief Initalise the sensor
 */
void init_bmp_280(void);

/*---------------------------------------------------------------------------*/
/**
 * \brief Enable/disable measurements
 * \param enable 0: disable, enable otherwise
 *
 * @return      none
 */
void enable_bmp_280(bool enable);

/*---------------------------------------------------------------------------*/
/**
 * \brief Read temperature and pressure data
 * \param data Pointer to a buffer where temperature and pressure will be
 *             written (6 bytes)
 * \return True if valid data could be retrieved
 */
bool read_data_bmp_280(uint8_t *data);

/*---------------------------------------------------------------------------*/
/**
 * \brief Convert raw data to values in degrees C (temp) and Pascal (pressure)
 * \param data Pointer to a buffer that holds raw sensor data
 * \param temp Pointer to a variable where the converted temperature will be
 *             written
 * \param press Pointer to a variable where the converted pressure will be
 *              written
 */
void nconvert(uint8_t *data, int32_t *temp, uint32_t *press);

/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param BMP_280_SENSOR_TYPE_TEMP or BMP_280_SENSOR_TYPE_PRESS
 * \return Temperature (centi degrees C) or Pressure (Pascal).
 */
int value_bmp_280(int type);

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
int configure_bmp_280(int enable);

/*---------------------------------------------------------------------------*/
#endif /* BMP_280_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
