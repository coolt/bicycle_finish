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
 * \defgroup sensortag-cc26xx-opt-sensor SensorTag 2.0 Light Sensor
 *
 * Due to the time required for the sensor to startup, this driver is meant to
 * be used in an asynchronous fashion. The caller must first activate the
 * sensor by calling SENSORS_ACTIVATE(). This will trigger the sensor's startup
 * sequence, but the call will not wait for it to complete so that the CPU can
 * perform other tasks or drop to a low power mode.
 *
 * Once the reading and conversion are complete, the driver will generate a
 * sensors_changed event.
 *
 * We use single-shot readings. In this mode, the hardware automatically goes
 * back to its shutdown mode after the conversion is finished. However, it will
 * still respond to I2C operations, so the last conversion can still be read
 * out.
 *
 * In order to take a new reading, the caller has to use SENSORS_ACTIVATE
 * again.
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26xx Opt3001 light sensor
 */
/*---------------------------------------------------------------------------*/
#ifndef OPT_3001_SENSOR_H_
#define OPT_3001_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor opt_3001_sensor;
/*---------------------------------------------------------------------------*/
/**
 * \brief Select the sensor on the I2C bus
 */
void select_opt_3001(void);

/*---------------------------------------------------------------------------*/
/**
 * \brief Turn the sensor on/off
 * \param enable TRUE: on, FALSE: off
 */
void enable_opt_3001(bool enable);

/*---------------------------------------------------------------------------*/
/**
 * \brief Read the result register
 * \param raw_data Pointer to a buffer to store the reading
 * \return TRUE if valid data
 */
bool read_data_opt_3001(uint16_t *raw_data);

/*---------------------------------------------------------------------------*/
/**
 * \brief Convert raw data to a value in lux
 * \param data Pointer to a buffer with a raw sensor reading
 * \return Converted value (lux)
 */
float convert_opt_3001(uint16_t raw_data);

/*---------------------------------------------------------------------------*/
/**
 * \brief Returns a reading from the sensor
 * \param type Ignored
 * \return Illuminance in centilux
 */
int value(int type);

/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the OPT3001 sensor.
 *
 * \param type Activate, enable or disable the sensor. See below
 * \param enable
 *
 * When type == SENSORS_HW_INIT we turn on the hardware
 * When type == SENSORS_ACTIVE and enable==1 we enable the sensor
 * When type == SENSORS_ACTIVE and enable==0 we disable the sensor
 */
int configure_opt_3001(int enable);

/*---------------------------------------------------------------------------*/
#endif /* OPT_3001_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
