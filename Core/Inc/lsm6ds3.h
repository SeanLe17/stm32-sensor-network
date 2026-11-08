#ifndef LSM6DS3_H
#define LSM6DS3_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} Lsm6ds3Raw;

bool lsm6ds3_init(uint8_t address_7bit);
bool lsm6ds3_read_who_am_i(uint8_t address_7bit, uint8_t *id);
bool lsm6ds3_read_motion(uint8_t address_7bit, Lsm6ds3Raw *output);

#endif