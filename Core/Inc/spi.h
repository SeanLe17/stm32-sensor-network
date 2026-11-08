#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi2_init(void);
uint8_t spi2_transfer_byte(uint8_t transmit);
void spi2_wait_idle(void);

#endif