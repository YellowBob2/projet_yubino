/* rng.c */
#include "rng.h"
#include <avr/io.h>

void rng_init(void) {
    // Configuration de l'ADC
    // REFS0 = 1 : Référence de voltage AVcc (5V)
    // MUX0 = 1  : Sélectionner le canal ADC1 (Broche A1) au lieu de A0
    //             (A0 est occupée par le bouton)
    ADMUX = (1 << REFS0) | (1 << MUX0); 
    
    // Activer l'ADC (ADEN) et régler le prescaler à 64 (ADPS2|ADPS1)
    // Fréquence ADC = 16MHz / 64 = 250kHz (dans la plage idéale 50-200kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); 
}

int rng_generate(uint8_t *dest, unsigned size) {
    for (unsigned i = 0; i < size; i++) {
        uint8_t byte = 0;
        for (uint8_t bit = 0; bit < 8; bit++) {
            ADCSRA |= (1 << ADSC); // Lancer une conversion
            while (ADCSRA & (1 << ADSC)); // Attendre la fin de la conversion
            
            // On récupère le bit de poids faible (LSB) du résultat (ADC)
            // Ce bit contient le "bruit thermique" et les imprécisions de mesure
            if (ADC & 1) {
                byte |= (1 << bit);
            }
        }
        dest[i] = byte;
    }
    return 1; // Succès
}