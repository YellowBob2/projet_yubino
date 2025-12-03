#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include "rng.h"


static volatile uint8_t adc_complete = 0;
static volatile uint8_t adc_result = 0;
static volatile uint16_t timer_noise = 0;

// rng avec ADC
static void rng_adc_init(void) {
    ADMUX = (1 << ADLAR);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);
}

ISR(ADC_vect) {
    adc_result = ADCH;
    adc_complete = 1;
}

static int rng_adc_generate(uint8_t* buffer, unsigned int size) {
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

// rng timer1
static void rng_timer_init(void) {
    TCCR1A = 0;
    TCCR1B = (1 << CS10);
    TIMSK1 = (1 << TOIE1);
    TCNT1 = 0;
}

ISR(TIMER1_OVF_vect) {
    timer_noise += TCNT1;
}

static int rng_timer_generate(uint8_t* buffer, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        timer_noise = 0;
        TCNT1 = 0;
        for (volatile uint16_t delay = 0; delay < 15000; delay++);
        buffer[i] = (uint8_t)timer_noise;
    }
    return size;
}

// mix des deux
static void rng_combined_init(void) {
    rng_adc_init();
    rng_timer_init();
    sei();
}

static int rng_combined_generate(uint8_t* buffer, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        // Byte ADC
        adc_complete = 0;
        ADCSRA |= (1 << ADSC);
        for (uint16_t timeout = 0; timeout < 500 && !adc_complete; timeout++) {
            _delay_us(100);
        }
        uint8_t adc_byte = adc_result;

        // Byte Timer
        timer_noise = 0;
        TCNT1 = 0;
        for (volatile uint16_t delay = 0; delay < 8000; delay++);
        uint8_t timer_byte = (uint8_t)timer_noise;

        // XOR des deux
        buffer[i] = adc_byte ^ timer_byte;
    }
    return size;
}


// #define RNG_METHOD_ADC       0
// #define RNG_METHOD_TIMER     1
// #define RNG_METHOD_COMBINED  2

typedef int (*rng_generator_t)(uint8_t*, unsigned int);
typedef void (*rng_init_t)(void);

struct {
    rng_init_t init;
    rng_generator_t generate;
} rng_current = {
    .init = rng_combined_init,
    .generate = rng_combined_generate
};

void rng_set_method(uint8_t method) {
    switch (method) {
        case RNG_METHOD_ADC:
            rng_current.init = rng_adc_init;
            rng_current.generate = rng_adc_generate;
            break;
        case RNG_METHOD_TIMER:
            rng_current.init = rng_timer_init;
            rng_current.generate = rng_timer_generate;
            break;
        case RNG_METHOD_COMBINED:
            rng_current.init = rng_combined_init;
            rng_current.generate = rng_combined_generate;
            break;
        default:
            method = RNG_METHOD_COMBINED;
    }
    rng_current.init();
}

void rng_init(void) {
    rng_set_method(RNG_METHOD_COMBINED);
}

int rng_generate(uint8_t* buffer, unsigned int size) {
    return rng_current.generate(buffer, size);
}
