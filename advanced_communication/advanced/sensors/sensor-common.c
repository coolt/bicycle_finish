/*
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-sensor-common
 * @{
 *
 * \file
 * Utilities common among SensorTag sensors
 */
/*---------------------------------------------------------------------------*/
#include "sensor-common.h"
#include "board-i2c.h"
/*---------------------------------------------------------------------------*/
/* Data to use when an error occurs */
#define ERROR_DATA                         0xCC
/*---------------------------------------------------------------------------*/
static uint8_t buffer[32];
/*---------------------------------------------------------------------------*/
bool
sensor_common_read_reg(uint8_t addr, uint8_t *buf, uint8_t len)
{
  return board_i2c_write_read(&addr, 1, buf, len);
}
/*---------------------------------------------------------------------------*/
bool
sensor_common_write_reg(uint8_t addr, uint8_t *buf, uint8_t len)
{
  uint8_t i;
  uint8_t *p = buffer;

  /* Copy address and data to local buffer for burst write */
  *p++ = addr;
  for(i = 0; i < len; i++) {
    *p++ = *buf++;
  }
  len++;

  /* Send data */
  return board_i2c_write(buffer, len);
}
/*---------------------------------------------------------------------------*/
void
sensor_common_set_error_data(uint8_t *buf, uint8_t len)
{
  while(len > 0) {
    len--;
    buf[len] = ERROR_DATA;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
