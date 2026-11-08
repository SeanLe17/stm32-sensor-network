#include "spi.h"

#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

void spi2_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    (void)RCC->AHB1ENR;
    (void)RCC->APB1ENR;

    GPIOB->MODER = (GPIOB->MODER & ~(0b11 << 26u)) | (0b10 << 26u);
    GPIOB->AFR[1] =(GPIOB->AFR[1] & ~(0b1111 << 20)) | (0b0101 << 20);

    GPIOB->MODER = (GPIOB->MODER & ~(0b11 << 28u)) | (0b10 << 28u);
    GPIOB->AFR[1] =(GPIOB->AFR[1] & ~(0b1111 << 24)) | (0b0101 << 24);

    GPIOB->MODER = (GPIOB->MODER & ~(0b11 << 30u)) | (0b10 << 30u);
    GPIOB->AFR[1] =(GPIOB->AFR[1] & ~(0b1111 << 28)) | (0b0101 << 28);

    GPIOB->PUPDR &=
    ~((0x3u << 26u) | (0x3u << 28u) | (0x3u << 30u));

    GPIOB->OTYPER &=
        ~((1u << 13u) | (1u << 14u) | (1u << 15u));

    GPIOB->OSPEEDR =
        (GPIOB->OSPEEDR &
        ~((0x3u << 26u) | (0x3u << 28u) | (0x3u << 30u))) |
        ((0x2u << 26u) | (0x2u << 28u) | (0x2u << 30u));

    SPI2->CR1 &= ~SPI_CR1_SPE;
    SPI2->CR1 |= SPI_CR1_MSTR;
    SPI2->CR1 |= SPI_CR1_CPOL;
    SPI2->CR1 |= SPI_CR1_CPHA;
    SPI2->CR1 |= SPI_CR1_BR_2;
    SPI2->CR1 |= SPI_CR1_SSM;
    SPI2->CR1 |= SPI_CR1_SSI;
    SPI2->CR2 = 0;

    SPI2->CR1 |= SPI_CR1_SPE;

}

uint8_t spi2_transfer_byte(uint8_t transmit)
{
    while ((SPI2->SR & SPI_SR_TXE) == 0u)
    {
    }
    *((volatile uint8_t *)&SPI2->DR) = transmit;

    while((SPI2->SR & SPI_SR_RXNE) == 0u)
    {
    }
    return *((volatile uint8_t *)&SPI2->DR);

}

void spi2_wait_idle(void)
{
    while ((SPI2->SR & SPI_SR_BSY) != 0u)
    {
    }
}