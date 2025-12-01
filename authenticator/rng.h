#ifndef RNG_H
#define RNG_H
#include <stdint.h>

void rng_init(void);
// Fonction de callback pour micro-ecc
int rng_generate(uint8_t *dest, unsigned size);

#endif