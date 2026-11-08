#ifndef I2C1_H
#define I2C1_H

#include <stdbool.h>
#include <stdint.h>

bool i2c1_init(void);

bool i2c1_write_register(uint8_t address_7bit,
                         uint8_t register_address,
                         uint8_t value);

bool i2c1_read_register(uint8_t address_7bit,
                        uint8_t register_address,
                        uint8_t *output);

#endif