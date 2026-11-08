#ifndef CAN_H
#define CAN_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t id;
    uint8_t length;
    uint8_t data[8];
} CanFrame;

bool can1_enter_init(void);

void can1_configure_loopback(void);

void can1_configure_filter(void);

bool can1_start(void);

bool can1_try_send(const CanFrame *frame);

bool can1_try_receive(CanFrame *out);
void can1_configure_500k_normal(void);

#endif