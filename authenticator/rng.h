#ifndef RNG_ADVANCED_H
#define RNG_ADVANCED_H

#include <stdint.h>

/* Méthodes RNG disponibles */
#define RNG_METHOD_LFSR       0   // LFSR 32-bit (rapide)
#define RNG_METHOD_ADC        1   // ADC bruité (aléatoire)
#define RNG_METHOD_COMBINED   2   // LFSR XOR ADC (équilibré) ← DÉFAUT
#define RNG_METHOD_TIMER      3   // Timer noise (très aléatoire)

#define F_CPU 16000000UL

void rng_lfsr_init(void);
int rng_lfsr_generate(uint8_t* buffer, unsigned int size);
void rng_adc_init(void);
int rng_adc_generate(uint8_t* buffer, unsigned int size);
void rng_combined_init(void);
int rng_combined_generate(uint8_t* buffer, unsigned int size);
void rng_timer_noise_init(void);
int rng_timer_generate(uint8_t* buffer, unsigned int size);

/**
 * Initialise le module RNG (utilise RNG_METHOD_COMBINED par défaut)
 */
void rng_init(void);

/**
 * Génère des octets aléatoires
 * @param buffer : destination
 * @param size : nombre d'octets à générer
 * @return : nombre d'octets générés (toujours == size)
 */
int rng_generate(uint8_t* buffer, unsigned int size);

/**
 * Change la méthode RNG au runtime
 * @param method : RNG_METHOD_* constant
 */
void rng_set_method(uint8_t method);

#endif // RNG_ADVANCED_H
