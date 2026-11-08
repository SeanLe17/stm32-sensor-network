#include "bme280.h"
#include "spi.h"
#include "stm32f446xx.h"
#include <stddef.h>
#include "stm32f4xx_hal.h"


static uint8_t bme280_read_register(uint8_t address)
{
    GPIOB->BSRR = (1u << 28u);
    uint8_t command = 0x80 | address;
    (void)spi2_transfer_byte(command);
    uint8_t received =spi2_transfer_byte(0x00);
    spi2_wait_idle();
    GPIOB->BSRR = (1u << 12u);
    return received;
}

uint8_t bme280_read_who_am_i()
{
    return bme280_read_register((0xD0u));
}

static void bme280_write_register(uint8_t address, uint8_t value)
{
    GPIOB->BSRR = (1u << 28u);
    uint8_t command = address & 0x7F;
    (void)spi2_transfer_byte(command);
    (void)spi2_transfer_byte(value);
    spi2_wait_idle();
    GPIOB->BSRR = (1u << 12u);
}

void bme280_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;

    /* Set PB12's output latch high before making it an output. */
    GPIOB->BSRR = (1u << 12u);

    /* PB12: normal GPIO output mode. */
    GPIOB->MODER =
        (GPIOB->MODER & ~(0x3u << 24u)) |
        (0x1u << 24u);

    /* PB12: push-pull, no pull resistor, high-speed output. */
    GPIOB->OTYPER &= ~(1u << 12u);
    GPIOB->PUPDR &= ~(0x3u << 24u);

    bme280_write_register(0xF2u, 0x01u);
    bme280_write_register(0xF4u, 0x25u);

    GPIOB->OSPEEDR =
        (GPIOB->OSPEEDR & ~(0x3u << 24u)) |
        (0x2u << 24u);
}




static void bme280_read_registers(uint8_t start_address, uint8_t *output, uint32_t count)
{
    GPIOB->BSRR = (1u << 28u);  // CS low

    uint8_t command = 0x80u | start_address;
    (void)spi2_transfer_byte(command); 

    for (uint32_t i = 0; i < count; i++)
    {
        output[i] = spi2_transfer_byte(0x00u);
    }

    spi2_wait_idle();
    GPIOB->BSRR = (1u << 12u);  // CS high
}

bool bme280_read_raw(uint32_t *pressure, uint32_t *temperature, uint16_t *humidity)
{
    if((pressure == NULL) | (temperature == NULL) | (humidity == NULL)){
        return false;
    }
    uint8_t raw[8];
    bme280_write_register(0x25, 0xF4);
    HAL_Delay(10);
    bme280_read_registers(0xF7u, raw, 8u);
    *pressure =((uint32_t)raw[0] << 12u) |((uint32_t)raw[1] << 4u)  | ((uint32_t)raw[2] >> 4u);
    *temperature =((uint32_t)raw[3] << 12u) | ((uint32_t)raw[4] << 4u)  | ((uint32_t)raw[5] >> 4u);
    *humidity = ((uint16_t)raw[6] << 8u) | (uint16_t)raw[7];

    return true;

}