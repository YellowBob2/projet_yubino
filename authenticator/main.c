/* main.c - PROJET FINAL YUBINO */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "uart.h"
#include "ui.h"
#include "rng.h"
#include "commands.h"

// Interruption UART utilisée uniquement pour réveiller le CPU du mode sleep.
// On ne lit PAS UDR0 ici pour ne pas "manger" l'octet que les fonctions
// de plus haut niveau (command_handle / uart_receive_byte) doivent traiter.
ISR(USART_RX_vect) {
    // Rien à faire : le simple fait d'avoir une ISR réveille le MCU.
}

int main(void) {
    uart_init();
    ui_init();
    rng_init();

    // Mode de sommeil "idle" : le CPU dort mais le module UART continue de fonctionner.
    set_sleep_mode(SLEEP_MODE_IDLE);
    sei(); // Autoriser les interruptions globales

    while (1) {
        // Endormir le CPU tant qu'aucun octet n'est disponible sur l'UART.
        while (!(UCSR0A & (1 << RXC0))) {
            sleep_mode();
        }

        // Lire l'octet de commande reçu
        uint8_t cmd = UDR0;

        // Déléguer le traitement au module de commandes, qui utilisera ensuite
        // uart_receive_byte() pour lire les éventuels octets suivants.
        command_handle(cmd);
    }

    return 0;
}
