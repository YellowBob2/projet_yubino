#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include "uart.h"
#include "ring_buffer.h"
#include "consts.h"
#include "ui.h"
#include "rng.h"
#include "storage.h"
#include "micro-ecc/uECC.h"
#include <avr/sleep.h>

// Buffers globaux
static uint8_t buffer_app_id[SHA1_APP_ID_SIZE];
static uint8_t buffer_challenge[CLIENT_DATA_HASH_SIZE];
static uint8_t public_key[PUBLIC_KEY_SIZE];
static uint8_t private_key[PRIVATE_KEY_SIZE];
static uint8_t credential_id[CREDENTIAL_ID_SIZE];
static uint8_t signature[SIGNATURE_SIZE];


uint8_t read_bytes_with_timeout(uint8_t* buffer, uint8_t length, uint16_t timeout_ms);
void send_error(uint8_t err_code);
void count_cb(uint8_t* cred_id, uint8_t* app_hash, void* data);
void send_cb(uint8_t* cred_id, uint8_t* app_hash, void* data);
void handle_make_credential(void);
void handle_get_assertion(void);
void handle_list_credentials(void);
void handle_reset(void);



uint8_t read_bytes_with_timeout(uint8_t* buffer, uint8_t length, uint16_t timeout_ms) {
    uint8_t bytes_read = 0;
    uint16_t elapsed = 0;

    while (bytes_read < length && elapsed < timeout_ms) {
        uint8_t data;
        if (UART__getbyte(&data) == 0) {
            buffer[bytes_read++] = data;
            elapsed = 0;  // reinit timeout si octet recu
        } else {
            _delay_ms(1);
            elapsed++;
        }
    }

    return (bytes_read == length) ? 1 : 0;
}

static void send_byte(uint8_t data) {
    UART__putbyte(data);
}

static void send_bytes(const uint8_t* data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        UART__putbyte(data[i]);
    }
}


void send_error(uint8_t err_code) {
    send_byte(err_code);
    send_byte(0);
}

void count_cb(uint8_t* cred_id, uint8_t* app_hash, void* data) {
    (void)cred_id; (void)app_hash;
    uint8_t* count_ptr = (uint8_t*)data;
    (*count_ptr)++;
}

void send_cb(uint8_t* cred_id, uint8_t* app_hash, void* data) {
    (void)data;
    send_bytes(cred_id, CREDENTIAL_ID_SIZE);
    send_bytes(app_hash, SHA1_APP_ID_SIZE);
}


// gestion des commandes

void handle_make_credential(void) {
    // Consentement
    if (!ui_wait_for_consent()) {
        send_error(STATUS_ERR_APPROVAL);
        return;
    }
    // generer cles
    uECC_set_rng(rng_generate);
    if (!uECC_make_key(public_key, private_key, uECC_secp160r1())) {
        send_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }
    // generer le credential id
    rng_generate(credential_id, CREDENTIAL_ID_SIZE);
    // sauvegarde dans l'eeprom le sha1 app_id, cred id et clé privee
    if (!storage_save(buffer_app_id, credential_id, private_key)) {
        send_byte(STATUS_ERR_STORAGE_FULL);
        return;
    }
    // MakeCredentialResponse:
    send_byte(STATUS_OK);
    send_bytes(credential_id, CREDENTIAL_ID_SIZE);
    send_bytes(public_key, PUBLIC_KEY_SIZE);
}


void handle_get_assertion(void) {
    // Chercher la clé
    if (!storage_find_key(buffer_app_id, private_key, credential_id)) {
        send_byte(STATUS_ERR_NOT_FOUND);
        return;
    }

    // Attendre consentement
    if (!ui_wait_for_consent()) {
        send_byte(STATUS_ERR_APPROVAL);
        return;
    }

    // Signer
    uECC_set_rng(rng_generate);
    if (!uECC_sign(private_key, buffer_challenge, CLIENT_DATA_HASH_SIZE, signature, uECC_secp160r1())) {
        send_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }

    // GetAssertionResponse:
    send_byte(STATUS_OK);
    send_bytes(credential_id, CREDENTIAL_ID_SIZE);
    send_bytes(signature, SIGNATURE_SIZE);
}


void handle_list_credentials() {
    uint8_t count = 0;
    storage_iterate(count_cb, &count);

    // ListCredentialsResponse:
    send_byte(STATUS_OK);
    send_byte(count);
    storage_iterate(send_cb, NULL);
}

void handle_reset() {
    if (!ui_wait_for_consent()) {
        send_byte(STATUS_ERR_APPROVAL);
        return;
    }
    storage_reset();
    // ResetResponse:
    send_byte(STATUS_OK);
}


int main(void) {
    UART__init();
    ui_init();
    rng_set_method(RNG_METHOD_COMBINED); // tester 3 aussi
    rng_init();
    //storage_init();
    set_sleep_mode(SLEEP_MODE_IDLE);

    while (1) {
        uint8_t cmd;

        if (UART__getbyte(&cmd) == 0) {
            switch (cmd) {

                case COMMAND_MAKE_CREDENTIAL: {
                    if (read_bytes_with_timeout(buffer_app_id, SHA1_APP_ID_SIZE, 1000) == 0) {
                        send_error(STATUS_ERR_BAD_PARAMETER);
                    } else {
                        handle_make_credential();
                    }
                    break;
                }

                case COMMAND_GET_ASSERTION: {
                    if (read_bytes_with_timeout(buffer_app_id, SHA1_APP_ID_SIZE, 1000) == 0 ||
                        read_bytes_with_timeout(buffer_challenge, CLIENT_DATA_HASH_SIZE, 1000) == 0) {
                        send_error(STATUS_ERR_BAD_PARAMETER);
                    } else {
                        handle_get_assertion();
                    }
                    break;
                }

                case COMMAND_LIST_CREDENTIALS: {
                    handle_list_credentials();
                    break;
                }

                case COMMAND_RESET: {
                    handle_reset();
                    break;
                }

                default: {
                    send_error(STATUS_ERR_COMMAND_UNKNOWN);
                    break;
                }
            }

        } else {
            UART__sleep();
        }
    }
    return 0;
}
