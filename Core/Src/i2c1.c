#include "i2c1.h"

#include <stddef.h>
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"

#define I2C_TIMEOUT_MS  20u

#define I2C_ERROR_FLAGS \
    (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR)

static void i2c1_clear_addr_flag(void)
{
    volatile uint32_t discard;

    discard = I2C1->SR1;
    discard = I2C1->SR2;
    (void)discard;
}

static void i2c1_abort(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
    I2C1->SR1 &= ~I2C_ERROR_FLAGS;
}

static bool i2c1_wait_sr1(uint32_t flag)
{
    uint32_t start = HAL_GetTick();

    while ((I2C1->SR1 & flag) == 0u)
    {
        if ((I2C1->SR1 & I2C_ERROR_FLAGS) != 0u)
        {
            i2c1_abort();
            return false;
        }

        if ((HAL_GetTick() - start) >= I2C_TIMEOUT_MS)
        {
            i2c1_abort();
            return false;
        }
    }

    return true;
}

static bool i2c1_wait_bus_idle(void)
{
    uint32_t start = HAL_GetTick();

    while ((I2C1->SR2 & I2C_SR2_BUSY) != 0u)
    {
        if ((HAL_GetTick() - start) >= I2C_TIMEOUT_MS)
        {
            return false;
        }
    }

    return true;
}

bool i2c1_init(void)
{
    /* Enable GPIOB and I2C1 clocks. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    (void)RCC->AHB1ENR;
    (void)RCC->APB1ENR;

    /*
     * PB6 = I2C1_SCL, PB7 = I2C1_SDA.
     * Alternate-function mode, AF4, open-drain, pull-up.
     */
    GPIOB->MODER =
        (GPIOB->MODER & ~((0x3u << 12u) | (0x3u << 14u))) |
        ((0x2u << 12u) | (0x2u << 14u));

    GPIOB->OTYPER |= (1u << 6u) | (1u << 7u);

    GPIOB->PUPDR =
        (GPIOB->PUPDR & ~((0x3u << 12u) | (0x3u << 14u))) |
        ((0x1u << 12u) | (0x1u << 14u));

    GPIOB->OSPEEDR =
        (GPIOB->OSPEEDR & ~((0x3u << 12u) | (0x3u << 14u))) |
        ((0x2u << 12u) | (0x2u << 14u));

    GPIOB->AFR[0] =
        (GPIOB->AFR[0] & ~((0xFu << 24u) | (0xFu << 28u))) |
        ((0x4u << 24u) | (0x4u << 28u));

    /*
     * APB1 runs at 42 MHz in your clock setup.
     * Standard-mode I2C: 100 kHz.
     */
    I2C1->CR1 = 0u;
    I2C1->CR2 = 42u;
    I2C1->CCR = 210u;
    I2C1->TRISE = 43u;
    I2C1->CR1 = I2C_CR1_PE | I2C_CR1_ACK;

    return true;
}

bool i2c1_write_register(uint8_t address_7bit,
                         uint8_t register_address,
                         uint8_t value)
{
    if (!i2c1_wait_bus_idle())
    {
        return false;
    }

    I2C1->CR1 |= I2C_CR1_START;

    if (!i2c1_wait_sr1(I2C_SR1_SB))
    {
        return false;
    }

    I2C1->DR = (uint8_t)(address_7bit << 1u);

    if (!i2c1_wait_sr1(I2C_SR1_ADDR))
    {
        return false;
    }

    i2c1_clear_addr_flag();

    I2C1->DR = register_address;

    if (!i2c1_wait_sr1(I2C_SR1_TXE))
    {
        return false;
    }

    I2C1->DR = value;

    if (!i2c1_wait_sr1(I2C_SR1_BTF))
    {
        return false;
    }

    I2C1->CR1 |= I2C_CR1_STOP;
    return true;
}

bool i2c1_read_register(uint8_t address_7bit,
                        uint8_t register_address,
                        uint8_t *output)
{
    if (output == NULL)
    {
        return false;
    }

    if (!i2c1_wait_bus_idle())
    {
        return false;
    }

    /* Begin write phase: select the device and its register pointer. */
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_START;

    if (!i2c1_wait_sr1(I2C_SR1_SB))
    {
        return false;
    }

    I2C1->DR = (uint8_t)(address_7bit << 1u);

    if (!i2c1_wait_sr1(I2C_SR1_ADDR))
    {
        return false;
    }

    i2c1_clear_addr_flag();

    I2C1->DR = register_address;

    if (!i2c1_wait_sr1(I2C_SR1_BTF))
    {
        return false;
    }

    /* Begin read phase: repeated START, same device, read direction. */
    I2C1->CR1 |= I2C_CR1_START;

    if (!i2c1_wait_sr1(I2C_SR1_SB))
    {
        return false;
    }

    I2C1->DR = (uint8_t)((address_7bit << 1u) | 1u);

    if (!i2c1_wait_sr1(I2C_SR1_ADDR))
    {
        return false;
    }

    /* One-byte receive: NACK it, issue STOP, then retrieve it. */
    I2C1->CR1 &= ~I2C_CR1_ACK;
    i2c1_clear_addr_flag();
    I2C1->CR1 |= I2C_CR1_STOP;

    if (!i2c1_wait_sr1(I2C_SR1_RXNE))
    {
        I2C1->CR1 |= I2C_CR1_ACK;
        return false;
    }

    *output = (uint8_t)I2C1->DR;

    I2C1->CR1 |= I2C_CR1_ACK;
    return true;
}