#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rng.h"

/* --- Configuration RNG multi-sources (ADC, Timer1, combiné) --- */

// Méthodes possibles
#define RNG_METHOD_ADC       0
#define RNG_METHOD_TIMER     1
#define RNG_METHOD_COMBINED  2

// États partagés entre ISR et générateurs
static volatile uint8_t adc_complete = 0;
static volatile uint8_t adc_result = 0;
static volatile uint16_t timer_noise = 0;

/* ---------- RNG basé sur l’ADC ---------- */

static void rng_adc_init(void) {
    // ADMUX :
    // - REFS0=1 : référence AVcc
    // - ADLAR=1 : alignement à gauche pour lire facilement ADCH
    ADMUX = (1 << REFS0) | (1 << ADLAR);

    // ADCSRA :
    // - ADEN : activer l’ADC
    // - ADPS2|ADPS1|ADPS0 : prescaler /128 pour fréquence ADC correcte
    // - ADIE : interruption ADC
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)
           | (1 << ADIE);
}

ISR(ADC_vect) {
    adc_result = ADCH;   // On ne garde que les bits de poids fort
    adc_complete = 1;
}

static int rng_adc_generate(uint8_t* buffer, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        adc_complete = 0;
        // Lancer une conversion
        ADCSRA |= (1 << ADSC);

        // Attendre la fin (avec timeout pour éviter blocage)
        for (uint16_t timeout = 0; timeout < 500 && !adc_complete; timeout++) {
            _delay_us(100);
        }

        buffer[i] = adc_result;
    }
    return (int)size;
}

/* ---------- RNG basé sur Timer1 ---------- */

static void rng_timer_init(void) {
    // Timer1 en mode normal, horloge sans préscaler
    TCCR1A = 0;
    TCCR1B = (1 << CS10);
    // Interruption sur overflow
    TIMSK1 = (1 << TOIE1);
    TCNT1 = 0;
}

ISR(TIMER1_OVF_vect) {
    // Accumule le bruit du timer dans timer_noise
    timer_noise += TCNT1;
}

static int rng_timer_generate(uint8_t* buffer, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        timer_noise = 0;
        TCNT1 = 0;
        // Attente "floue" pour injecter de la gigue
        for (volatile uint16_t delay = 0; delay < 15000; delay++);
        buffer[i] = (uint8_t)timer_noise;
    }
    return (int)size;
}

/* ---------- RNG combiné ADC + Timer1 ---------- */

static void rng_combined_init(void) {
    rng_adc_init();
    rng_timer_init();
    sei(); // Nécessaire pour activer les ISR ADC et TIMER1
}

static int rng_combined_generate(uint8_t* buffer, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        /* Byte ADC */
        adc_complete = 0;
        ADCSRA |= (1 << ADSC);
        for (uint16_t timeout = 0; timeout < 500 && !adc_complete; timeout++) {
            _delay_us(100);
        }
        uint8_t adc_byte = adc_result;

        /* Byte Timer */
        timer_noise = 0;
        TCNT1 = 0;
        for (volatile uint16_t delay = 0; delay < 8000; delay++);
        uint8_t timer_byte = (uint8_t)timer_noise;

        /* Mix des deux sources */
        buffer[i] = adc_byte ^ timer_byte;
    }
    return (int)size;
}

/* ---------- Sélection dynamique de la méthode ---------- */

typedef int (*rng_generator_t)(uint8_t*, unsigned int);
typedef void (*rng_init_t)(void);

static struct {
    rng_init_t init;
    rng_generator_t generate;
} rng_current = {
    .init = rng_combined_init,
    .generate = rng_combined_generate
};

// Si à l’avenir tu veux changer de méthode à chaud, tu peux exposer cette
// fonction dans rng.h. Pour le moment, on reste en scope interne.
static void rng_set_method(uint8_t method) {
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
        default:
            rng_current.init = rng_combined_init;
            rng_current.generate = rng_combined_generate;
            break;
    }
    rng_current.init();
}

void rng_init(void) {
    rng_set_method(RNG_METHOD_COMBINED);
}

int rng_generate(uint8_t* buffer, unsigned int size) {
    return rng_current.generate(buffer, size);
}
