/*
 * Humidity sensor
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-peripherals
 * @{
 *
 * \defgroup sensortag-cc26xx-hdc-sensor SensorTag 2.0 TI HDC1000 Sensor
 *
 * Due to the time required for the sensor to startup, this driver is meant to
 * be used in an asynchronous fashion. The caller must first activate the
 * sensor by calling SENSORS_ACTIVATE(). This will trigger the sensor's startup
 * sequence, but the call will not wait for it to complete so that the CPU can
 * perform other tasks or drop to a low power mode. Once the sensor has taken
 * readings, it will automatically go back to low power mode.
 *
 * Once the sensor is stable, the driver will retrieve readings from the sensor
 * and latch them. It will then generate a sensors_changed event.
 *
 * The user can then retrieve readings by calling .value() and by passing
 * either HDC_1000_SENSOR_TYPE_TEMP or HDC_1000_SENSOR_TYPE_HUMIDITY as the
 * argument. Multiple calls to value() will not trigger new readings, they will
 * simply return the most recent latched values.
 *
 * The user can query the sensor's status by calling .status()
 *
 * To get a fresh reading, the user must trigger a new reading cycle by calling
 * SENSORS_ACTIVATE().
 * @{
 *
 * \file
 * Header file for the Sensortag-CC26ss TI HDC1000 sensor
 */
/*---------------------------------------------------------------------------*/
#ifndef HDC_1000_SENSOR_H
#define HDC_1000_SENSOR_H
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/*---------------------------------------------------------------------------*/
#define HDC_1000_SENSOR_TYPE_TEMP        1
#define HDC_1000_SENSOR_TYPE_HUMIDITY    2
/*---------------------------------------------------------------------------*/
/**
 * \name HDC1000 driver states
 * @{
 */
#define HDC_1000_SENSOR_STATUS_DISABLED         0 /**< Not initialised */
#define HDC_1000_SENSOR_STATUS_INITIALISED      1 /**< Initialised but idle */
#define HDC_1000_SENSOR_STATUS_TAKING_READINGS  2 /**< Readings in progress */
#define HDC_1000_SENSOR_STATUS_READINGS_READY   3 /**< Both readings ready */
/** @} */
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor hdc_1000_sensor;

 /*---------------------------------------------------------------------------*/
 /**
  * \brief       Initialise the humidity sensor driver
  * \return      True if I2C operation successful
  */
 bool init_hdc_1000(void);

 /*---------------------------------------------------------------------------*/
 /**
  * \brief       Start measurement
  */
 void start_hdc_1000(void);

 /*---------------------------------------------------------------------------*/
 /**
  * \brief       Take readings from the sensor
  * \return      true of I2C operations successful
  */
 bool read_data_hdc_1000();

 /*---------------------------------------------------------------------------*/
 /**
  * \brief       Convert raw data to temperature and humidity
  * \param       temp - converted temperature
  * \param       hum - converted humidity
  */
 void convert_hdc_1000(float *temp, float *hum);


 /*---------------------------------------------------------------------------*/
 /**
  * \brief Returns a reading from the sensor
  * \param type HDC_1000_SENSOR_TYPE_TEMP or HDC_1000_SENSOR_TYPE_HUMIDITY
  * \return Temperature (centi degrees C) or Humidity (centi %RH)
  */
 int value_hdc_1000(int type);

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
 void configure_hdc_1000(void);


/*---------------------------------------------------------------------------*/
#endif /* HDC_1000_SENSOR_H */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
