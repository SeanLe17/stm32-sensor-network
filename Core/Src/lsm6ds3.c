#include "lsm6ds3.h"

#include <stddef.h>
#include "i2c1.h"

#define LSM6DS3_WHO_AM_I      0x0Fu

#define LSM6DS3_CTRL1_XL      0x10u
#define LSM6DS3_CTRL2_G       0x11u
#define LSM6DS3_CTRL3_C       0x12u

#define LSM6DS3_OUTX_L_G      0x22u
#define LSM6DS3_OUTX_L_XL     0x28u

#define LSM6DS3_WHO_AM_I_ID   0x69u

static bool lsm6ds3_read_i16(uint8_t address_7bit,
                              uint8_t low_register,
                              int16_t *output)
{
    uint8_t low_byte;
    uint8_t high_byte;

    if (output == NULL)
    {
        return false;
    }

    if (!i2c1_read_register(address_7bit, low_register, &low_byte))
    {
        return false;
    }

    if (!i2c1_read_register(address_7bit,
                            (uint8_t)(low_register + 1u),
                            &high_byte))
    {
        return false;
    }

    *output = (int16_t)(
        (uint16_t)low_byte |
        ((uint16_t)high_byte << 8u)
    );

    return true;
}

bool lsm6ds3_read_who_am_i(uint8_t address_7bit, uint8_t *id)
{
    if (id == NULL)
    {
        return false;
    }

    return i2c1_read_register(address_7bit, LSM6DS3_WHO_AM_I, id);
}

bool lsm6ds3_init(uint8_t address_7bit)
{
    uint8_t id;

    if (!lsm6ds3_read_who_am_i(address_7bit, &id))
    {
        return false;
    }

    if (id != LSM6DS3_WHO_AM_I_ID)
    {
        return false;
    }

    /*
     * CTRL3_C:
     * BDU = 1: data registers do not change halfway through a two-byte read.
     * IF_INC = 1: internal register address auto-increments for future bursts.
     */
    if (!i2c1_write_register(address_7bit, LSM6DS3_CTRL3_C, 0x44u))
    {
        return false;
    }

    /* Accelerometer: 104 Hz, ±2 g. */
    if (!i2c1_write_register(address_7bit, LSM6DS3_CTRL1_XL, 0x40u))
    {
        return false;
    }

    /* Gyroscope: 104 Hz, ±245 dps. */
    if (!i2c1_write_register(address_7bit, LSM6DS3_CTRL2_G, 0x40u))
    {
        return false;
    }

    return true;
}

bool lsm6ds3_read_motion(uint8_t address_7bit, Lsm6ds3Raw *output)
{
    if (output == NULL)
    {
        return false;
    }

    if (!lsm6ds3_read_i16(address_7bit, LSM6DS3_OUTX_L_G, &output->gyro_x))
    {
        return false;
    }

    if (!lsm6ds3_read_i16(address_7bit, LSM6DS3_OUTX_L_G + 2u, &output->gyro_y))
    {
        return false;
    }

    if (!lsm6ds3_read_i16(address_7bit, LSM6DS3_OUTX_L_G + 4u, &output->gyro_z))
    {
        return false;
    }

    if (!lsm6ds3_read_i16(address_7bit, LSM6DS3_OUTX_L_XL, &output->accel_x))
    {
        return false;
    }

    if (!lsm6ds3_read_i16(address_7bit, LSM6DS3_OUTX_L_XL + 2u, &output->accel_y))
    {
        return false;
    }

    if (!lsm6ds3_read_i16(address_7bit, LSM6DS3_OUTX_L_XL + 4u, &output->accel_z))
    {
        return false;
    }

    return true;
}