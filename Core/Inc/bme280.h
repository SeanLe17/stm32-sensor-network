#ifndef BME280_H
#define BME280_H
#include <stdbool.h>
#include <stdint.h>

void bme280_init(void);
uint8_t bme280_read_who_am_i(void);
bool bme280_read_raw(uint32_t *pressure, uint32_t *temperature, uint16_t *humidity);
#endif