#ifndef RNG_ADVANCED_H
#define RNG_ADVANCED_H

#include <stdint.h>

#define RNG_METHOD_ADC       0
#define RNG_METHOD_TIMER     1
#define RNG_METHOD_COMBINED  2

#define F_CPU 16000000UL

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
