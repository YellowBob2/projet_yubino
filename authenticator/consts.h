#ifndef CONSTS_H
#define CONSTS_H

#include <stdint.h>

// types de commandes
#define COMMAND_LIST_CREDENTIALS 0x00
#define COMMAND_MAKE_CREDENTIAL 0x01
#define COMMAND_GET_ASSERTION 0x02
#define COMMAND_RESET 0x03

// types de status
#define STATUS_OK 0x00
#define STATUS_ERR_COMMAND_UNKNOWN 0x01
#define STATUS_ERR_CRYPTO_FAILED 0x02
#define STATUS_ERR_BAD_PARAMETER 0x03
#define STATUS_ERR_NOT_FOUND 0x04
#define STATUS_ERR_STORAGE_FULL 0x05
#define STATUS_ERR_APPROVAL 0x06

// tailles des elems dans les requetes/reponses
#define SHA1_APP_ID_SIZE 20
#define CREDENTIAL_ID_SIZE 16
#define PUBLIC_KEY_SIZE 40
#define PRIVATE_KEY_SIZE 21
#define SIGNATURE_SIZE 40
#define CLIENT_DATA_HASH_SIZE 20

// Configuration UI
#define LED_BLINK_INTERVAL_MS 500   // 0.5 sec pour le clignotement led
#define CONSENT_TIMEOUT_MS 10000 // 10 sec d'attente du consentement

// Constantes matérielles
#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_PIN 5 // PB5

#define BUTTON_PORT PORTD
#define BUTTON_PIN PIND
#define BUTTON_DDR DDRD
#define BUTTON_NUM  2 // PD2

#endif // CONSTS_H
