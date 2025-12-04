#include "uart.h"
#include <avr/io.h>

#define BAUD 115200
#define USE_2X 1
#include <util/setbaud.h>

void uart_init(void) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    
    #if USE_2X
    UCSR0A |= (1 << U2X0);
    #else
    UCSR0A &= ~(1 << U2X0);
    #endif

    // 8 bits de données, pas de parité, 1 stop bit (8N1) 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Activer RX, TX et interruption de réception (pour réveiller le MCU du mode sleep)
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

void uart_send_byte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_send_buffer(const uint8_t* data, uint16_t len) {
    for(uint16_t i = 0; i < len; i++) {
        uart_send_byte(data[i]);
    }
}

uint8_t uart_receive_byte(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}