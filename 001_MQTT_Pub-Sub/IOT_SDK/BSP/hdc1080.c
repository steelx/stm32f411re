/*
 * hdc1080.c
 *
 *  Created on: Feb 17, 2025
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 *
 *  STUB: this file compiles without HAL, but the HDC1080 sensor is not
 *  actually accessed.  Add a register-level I2C1 transmit/receive layer
 *  and re-implement these functions if you need the sensor.
 */

#include "hdc1080.h"
#include <stddef.h>

hdc_status_t hdc1080_init(void *hi2c_x,
    temp_resolution_t temp_resolution_x_bit,
    humi_resolution_t humi_resolution_x_bit) {
  (void)hi2c_x;
  (void)temp_resolution_x_bit;
  (void)humi_resolution_x_bit;
  return HDC_ERROR;
}

hdc_status_t hdc1080_read_temperature(void *hi2c_x, float *temperature) {
  (void)hi2c_x;
  if (temperature != NULL) {
    *temperature = 0.0f;
  }
  return HDC_ERROR;
}

hdc_status_t hdc1080_read_humidity(void *hi2c_x, uint8_t *humidity) {
  (void)hi2c_x;
  if (humidity != NULL) {
    *humidity = 0;
  }
  return HDC_ERROR;
}

hdc_status_t hdc1080_start_measurement(void *hi2c_x,
    float *temperature, uint8_t *humidity) {
  (void)hi2c_x;
  if (temperature != NULL) {
    *temperature = 0.0f;
  }
  if (humidity != NULL) {
    *humidity = 0;
  }
  return HDC_ERROR;
}
