#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include "rng.h"

// 1 lfsr (pas tres bien en tout cas pas pour les clés)

static uint32_t lfsr_state = 0x12345678;

void rng_lfsr_init(void) {
    ADMUX = 0;
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Seed le LFSR avec plusieurs lectures ADC
    for (int i = 0; i < 8; i++) {
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        lfsr_state = (lfsr_state << 8) | ADCH;
    }
}

int rng_lfsr_generate(uint8_t* buffer, unsigned int size) {
    // LFSR Galois 32 bits
    for (unsigned int i = 0; i < size; i++) {
        uint8_t lsb = lfsr_state & 1;
        lfsr_state >>= 1;
        if (lsb) {
            lfsr_state ^= 0xB4000000;  // avec ce polynome
        }
        buffer[i] = (uint8_t)(lfsr_state >> 24);
    }
    return size;
}

// 2 ADC (mieux)

static volatile uint8_t adc_complete = 0;
static volatile uint8_t adc_result = 0;

void rng_adc_init(void) {
    ADMUX = (1 << ADLAR);
    // Prescaler 128 (125 kHz pour 16 MHz clock)
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2)
           | (1 << ADPS1)
           | (1 << ADPS0)
           | (1 << ADIE);
}

// Interruption ADC complété
ISR(ADC_vect) {
    adc_result = ADCH;
    adc_complete = 1;
}

int rng_adc_generate(uint8_t* buffer, unsigned int size) {

    for (unsigned int i = 0; i < size; i++) {
        adc_complete = 0;
        ADCSRA |= (1 << ADSC);

        for (uint16_t timeout = 0; timeout < 500 && !adc_complete; timeout++) {
            _delay_us(100);
        }

        buffer[i] = adc_result;
    }
    return size;
}

// 3 mix des deux


void rng_combined_init(void) {
    rng_lfsr_init();
    rng_adc_init();
}

int rng_combined_generate(uint8_t* buffer, unsigned int size) {

    //uint8_t adc_byte;
    for (unsigned int i = 0; i < size; i++) {
        uint8_t lsb = lfsr_state & 1;
        lfsr_state >>= 1;
        if (lsb) {
            lfsr_state ^= 0xB4000000;
        }
        uint8_t lfsr_byte = (uint8_t)(lfsr_state >> 24);

        adc_complete = 0;
        ADCSRA |= (1 << ADSC);
        for (uint16_t timeout = 0; timeout < 500 && !adc_complete; timeout++) {
            _delay_us(100);
        }

        buffer[i] = lfsr_byte ^ adc_result;
    }
    return size;
}


// 4 le timer (par contre c'est lent)

// avec timer 1 car timer0 utilisé pour ui.c
static volatile uint16_t timer_noise = 0;

void rng_timer_noise_init(void) {
    TCCR1A = 0;
    TCCR1B = (1 << CS10);
    TIMSK1 = (1 << TOIE1);
}

ISR(TIMER1_OVF_vect) {
    timer_noise += TCNT1;
}

int rng_timer_generate(uint8_t* buffer, unsigned int size) {

    for (unsigned int i = 0; i < size; i++) {
        timer_noise = 0;

        // Attendre plusieurs interruptions timer
        for (volatile uint16_t delay = 0; delay < 10000; delay++);

        buffer[i] = (uint8_t)timer_noise;
    }
    return size;
}


typedef int (*rng_generator_t)(uint8_t*, unsigned int);
typedef void (*rng_init_t)(void);

struct {
    rng_init_t init;
    rng_generator_t generate;
} rng_current = {
    .init = rng_combined_init,
    .generate = rng_combined_generate
};

/**
 * Changer la méthode RNG au runtime :
 *
 * rng_set_method(RNG_METHOD_LFSR);     // Rapide, peu aléatoire
 * rng_set_method(RNG_METHOD_ADC);      // Lent, très aléatoire
 * rng_set_method(RNG_METHOD_COMBINED); // Équilibre
 * rng_set_method(RNG_METHOD_TIMER);    // Très lent, maximum entropie
 */

// #define RNG_METHOD_LFSR       0
// #define RNG_METHOD_ADC        1
// #define RNG_METHOD_COMBINED   2
// #define RNG_METHOD_TIMER      3

void rng_set_method(uint8_t method) {
    switch (method) {
        case RNG_METHOD_LFSR:
            rng_current.init = rng_lfsr_init;
            rng_current.generate = rng_lfsr_generate;
            break;
        case RNG_METHOD_ADC:
            rng_current.init = rng_adc_init;
            rng_current.generate = rng_adc_generate;
            break;
        case RNG_METHOD_COMBINED:
            rng_current.init = rng_combined_init;
            rng_current.generate = rng_combined_generate;
            break;
        case RNG_METHOD_TIMER:
            rng_current.init = rng_timer_noise_init;
            rng_current.generate = rng_timer_generate;
            break;
    }
    rng_current.init();
}


void rng_init(void) {
    rng_current.init();
}

int rng_generate(uint8_t* buffer, unsigned int size) {
    return rng_current.generate(buffer, size);
}
