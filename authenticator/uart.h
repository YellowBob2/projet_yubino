#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART__init(void);
uint8_t UART__getbyte(uint8_t *data);
void UART__putbyte(uint8_t data);
void UART__sleep(void);

#endif
