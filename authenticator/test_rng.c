#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "rng.h"

#define BYTE_RANGE 256

// test de la qualité du générateur
// on veux se raprocher d'une entropie de 8 bits/byte qui correspond a un tirage uniforme
// ie on aura un bon aléa


// tester avec
// avr-gcc -mmcu=atmega328p -Os -o test_entropy.elf test_entropy.c rng.c
// avr-objcopy -O ihex -R .eeprom test_entropy.elf test_entropy.hex
// avrdude -c usbtiny -p m328p -U flash:w:test_entropy.hex


// on va generer un echantillon de nombres aleatoires generers par la fonction
// rng chosie et on calcule l'entropie
double entropy(const uint8_t* data, unsigned int size) {
    unsigned int freq[BYTE_RANGE] = {0};
    for (unsigned int i = 0; i < size; i++) {
        freq[data[i]]++;
    }

    double ent = 0.0;
    for (int i = 0; i < BYTE_RANGE; i++) {
        if (freq[i] > 0) {
            double p = (double)freq[i] / size;
            ent -= p * log2(p);
        }
    }
    return ent;
}

int main() {
    uint8_t buffer[1024];
    //rng_set_method(RNG_METHOD_COMBINED); //choisir la maethode test
    rng_init();
    rng_generate(buffer, sizeof(buffer));

    double ent = entropy(buffer, sizeof(buffer));
    printf("Entropy of generated data: %f bits /byte\n", ent);
    return 0;
}
