/* ui.c */
#include "ui.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// Décommenter pour activer le mode debug (auto-consent rapide)
// #define UI_AUTOCONSENT_DEBUG

// LED pilotée par la Pin D5 (PD5)
#define LED_PIN         PD5
// Bouton sur la Pin A0 (PC0)
#define BTN_PIN         PC0

static volatile uint16_t ui_tick_10ms = 0;
static volatile uint8_t ui_tick_flag = 0;

void ui_timer0_init(void) {
    // CTC
    TCCR0A = (1 << WGM01);
    // Prescaler 64
    TCCR0B = (1 << CS01) | (1 << CS00);
    // 250 kHz → 250 cycles = 1 ms
    OCR0A = 249;
    // Interruption
    TIMSK0 = (1 << OCIE0A);
}


ISR(TIMER0_COMPA_vect) {
    static uint8_t div10 = 0;

    div10++;
    if (div10 >= 10) {   // 10 ms
        div10 = 0;
        ui_tick_flag = 1;
        ui_tick_10ms++;
    }
}



void ui_init(void) {
    DDRD |= (1 << LED_PIN);
    PORTD &= ~(1 << LED_PIN);

    DDRC &= ~(1 << BTN_PIN);
    PORTC |= (1 << BTN_PIN);

    ui_timer0_init();
}


uint8_t ui_wait_for_consent(void) {
    ui_tick_10ms = 0;
    ui_tick_flag = 0;
    PORTD &= ~(1 << LED_PIN);

    while (ui_tick_10ms < 1000) { // 10 secondes

        // Réveil toutes les 10 ms par Timer0
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();

        if (ui_tick_flag) {
            ui_tick_flag = 0;

            // Bouton appuyé ?
            if (!(PINC & (1 << BTN_PIN))) {
                PORTD &= ~(1 << LED_PIN);
                return 1;
            }

            // Clignotement toutes les 500 ms
            if ((ui_tick_10ms % 50) == 0) {
                PORTD ^= (1 << LED_PIN);
            }

            #ifdef UI_AUTOCONSENT_DEBUG
            if (ui_tick_10ms >= 50) {
                PORTD &= ~(1 << LED_PIN);
                return 1;
            }
            #endif
        }
    }

    PORTD &= ~(1 << LED_PIN);
    return 0;
}
