#include "ui.h"
#include "consts.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <avr/sleep.h>

#define F_CPU 16000000UL

static volatile uint16_t g_ms_tick = 0;
static volatile uint8_t g_button_confirmed = 0;
static volatile uint8_t g_led_pwm = 0;

// ISR avec timer0
ISR(TIMER0_OVF_vect) {
    g_ms_tick++;
}

ISR(INT0_vect) {
}

// ms écoulées depuis reset compteur
uint16_t ui_get_ms(void) {
    uint16_t copy;
    cli();
    copy = g_ms_tick;
    sei();
    return copy;
}

// Reset compteur ms
static void ui_reset_ms(void) {
    cli();
    g_ms_tick = 0;
    sei();
}

// 0 = bouton appuye, 1 relache
static uint8_t ui_button_is_pressed_raw(void) {
    return (BUTTON_PIN & (1 << BUTTON_NUM)) == 0;
}

// Initialisation UI: LED + bouton + Timer0 PWM + Timer0 overflow + INT0
void ui_init(void) {
    LED_DDR |= (1 << LED_PIN);
    BUTTON_DDR &= ~(1 << BUTTON_NUM);
    BUTTON_PORT |= (1 << BUTTON_NUM);
    // Configurer Timer0 en Fast PWM, non-inverting sur OC0A, prescaler 64
    TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
    TCCR0B = (1 << CS01) | (1 << CS00);
    // led etteinte
    OCR0A = 0;
    TIMSK0 |= (1 << TOIE0);
    // Configurer INT0 sur front descendant (bouton)
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
    sei();
}


// on dort jusqu'au prochain tick Timer0 ou interr
void ui_sleep_tick(uint16_t last_ms) {
    cli();
    if (g_ms_tick == last_ms) {
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
    } else {
        sei();
    }
}


// on att le consentement = qu'on appuie sur boutton
// pdt ce tps on fait clignoter la led a 1Hz = 500ms on + 500ms off
uint8_t ui_wait_for_consent(void) {
    uint16_t last_ms = 0;
    uint16_t blink_elapsed = 0; // temps depuis dernier toggle led
    uint16_t debounce_elapsed = 0; // temps depuis changement etat bouton
    uint8_t  led_on = 0;
    uint8_t  last_button_state = ui_button_is_pressed_raw();

    g_button_confirmed = 0;
    ui_reset_ms();
    last_ms = ui_get_ms();

    led_on = 1;
    OCR0A = 255; //itensite max

    while (ui_get_ms() < CONSENT_TIMEOUT_MS) {
        uint16_t now = ui_get_ms();
        uint16_t delta = now - last_ms;
        if (delta == 0) {
            ui_sleep_tick(last_ms);
            continue;
        }
        last_ms = now;
        blink_elapsed += delta;
        debounce_elapsed += delta;

        if (blink_elapsed >= 500) {
            blink_elapsed = 0;
            led_on = !led_on;
            if (led_on) {
                OCR0A = 255;
            } else {
                OCR0A = 0;
            }
        }

        // on regarde le bouton (lit etat et debounce)
        // si etat change on reset debounce, sinon on valide l'appuie bouton
        uint8_t cur_state = ui_button_is_pressed_raw();
        if (cur_state != last_button_state) {
            last_button_state = cur_state;
            debounce_elapsed = 0;
        } else {
            if (cur_state == 0 && debounce_elapsed >= 20) {
                g_button_confirmed = 1;
            }
        }

        if (g_button_confirmed) {
            OCR0A = 255;
            return 1; // consentement donné
        }

        // on dort jusquau prochain tick ou bouton
        ui_sleep_tick(last_ms);
    }


    OCR0A = 0;
    return 0;
}
