#ifndef UART_H
#define UART_H
#include <stdbool.h>

void uart2_init(void);
void uart2_write_char(char c);
bool uart2_pop(char *out);
void uart2_write_string(const char *text);

#endif