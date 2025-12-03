#include "uart.h"
#include "ring_buffer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define FOSC 16000000UL
#define BAUD 115200
#define UBRR ((FOSC/16/BAUD)- 1)
#define BUFFER_SIZE 128

static uint8_t uart_rx_buffer[BUFFER_SIZE];
static struct ring_buffer rx_buffer;

void UART__init(){
    // set baud rate dans registre ubrr0
    UBRR0H = (unsigned char)(UBRR >> 8);
    UBRR0L = (unsigned char)UBRR;
    // active tx transmission, rx reception et rx interrupt
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    // 8 bits de data et 1 bit stop sans bit de parité
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // init le buffer et les interruptions
    ring_buffer__init(&rx_buffer, uart_rx_buffer, BUFFER_SIZE);
    // mode sleep et interruptions globales
    set_sleep_mode(SLEEP_MODE_IDLE);
    sei();
}

uint8_t UART__getbyte(uint8_t* data) {
    return ring_buffer__pop(&rx_buffer, data);
}


void UART__putbyte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}


ISR(USART_RX_vect) {
    uint8_t c = UDR0;
    ring_buffer__push(&rx_buffer, c);
}

// dormir en attendant de recevoir des donnees
void UART__sleep(void) {
    cli();
    // si y'a plus de données dans le buffer -> pas de commandes recues donc on dort
    if (rx_buffer.head ==rx_buffer.tail) {
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    } else {
        sei();
    }
}
