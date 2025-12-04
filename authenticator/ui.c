/* ui.c */
#include "ui.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

// Décommenter pour activer le mode debug (auto-consent rapide)
// #define UI_AUTOCONSENT_DEBUG

// LED pilotée par la Pin D5 (PD5)
#define LED_PIN         PD5
// Bouton sur la Pin A0 (PC0)
#define BTN_PIN         PC0

void ui_init(void) {
    /* Configuration de la LED sur D5 */
    DDRD |= (1 << LED_PIN);   // PD5 en SORTIE
    PORTD &= ~(1 << LED_PIN); // Initialiser à LOW 

    /* Configuration du Bouton sur A0 */
    // Configurer A0 en ENTRÉE
    DDRC &= ~(1 << BTN_PIN);
    // Activer la résistance de PULL-UP interne sur A0
    // Nécessaire car le bouton connecte au GND
    PORTC |= (1 << BTN_PIN);
}

uint8_t ui_wait_for_consent(void) {
    uint16_t timeout_counter = 0;

    // Boucle de 10 secondes (1000 * 10ms)
    while (timeout_counter < 1000) {

        // Lire le bouton sur A0 (PC0)
        // PINC lit l'état réel des broches du Port C
        // Si le bouton est appuyé (connecté au GND), le bit est à 0
        if (!(PINC & (1 << BTN_PIN))) {
            PORTD &= ~(1 << LED_PIN); // Éteindre la LED avant de sortir
            return 1; // Succès (Consentement donné)
        }

        // Mode debug : auto-consent après ~500 ms pour ne pas bloquer les tests
        #ifdef UI_AUTOCONSENT_DEBUG
        if (timeout_counter >= 50) { // 50 * 10 ms = 500 ms
            PORTD &= ~(1 << LED_PIN);
            return 1;
        }
        #endif

        // Gestion du clignotement (Toggle)
        // Change d'état toutes les 500ms (50 * 10ms)
        if ((timeout_counter % 50) == 0) {
            PORTD ^= (1 << LED_PIN); // Inverse l'état de D5
        }

        // Pour limiter la consommation, on peut laisser le CPU dormir entre
        // deux échantillons bouton. Le réveil est assuré par les interruptions
        // déjà actives (UART RX, Timer1, ADC).
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();

        // Petite temporisation de sécurité pour éviter de sur-échantillonner
        // le bouton et garder un clignotement régulier.
        _delay_ms(10);
        timeout_counter++;
    }

    PORTD &= ~(1 << LED_PIN); // S'assurer que la LED est éteinte en cas de timeout
    return 0; // Echec (Timeout)
}