#ifndef UART_H
#define UART_H
#include <stdint.h>

void uart_init(void);
void uart_send_byte(uint8_t data);
void uart_send_buffer(const uint8_t* data, uint16_t len);
uint8_t uart_receive_byte(void);

#endif