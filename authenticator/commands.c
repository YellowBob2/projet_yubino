#include <stdio.h>

#include "commands.h"
#include "uart.h"
#include "ring_buffer.h"
#include "consts.h"
#include "ui.h"
#include "rng.h"
#include "storage.h"
#include "micro-ecc/uECC.h"
#include "globals.h"


uint8_t read_bytes_with_timeout(uint8_t* buffer, uint8_t length, uint16_t timeout_ms) {
    uint8_t bytes_read = 0;
    uint16_t start_ms = ui_get_ms();

    while (bytes_read < length) {
        uint16_t now_ms = ui_get_ms();
        uint16_t elapsed = now_ms - start_ms;

        if (elapsed >= timeout_ms) {
            return 0;
        }

        uint8_t data;
        if (UART__getbyte(&data) == 0) {
            buffer[bytes_read++] = data;
            start_ms = now_ms;
        }
        ui_sleep_tick(now_ms);  // reutilise le timer0 qui incremente tick dans ui.c
    }
    return 1;
}

// static uint8_t read_bytes_with_timeout(uint8_t* buffer, uint8_t length, uint16_t timeout_ms) {
//     uint8_t bytes_read = 0;
//     uint16_t elapsed = 0;
//
//     while (bytes_read < length && elapsed < timeout_ms) {
//         uint8_t data;
//         if (UART__getbyte(&data) == 0) {
//             buffer[bytes_read++] = data;
//             elapsed = 0;  // reinit timeout si octet recu
//         } else {
//             _delay_ms(1);
//             elapsed++;
//         }
//     }
//
//     return (bytes_read == length) ? 1 : 0;
// }


void send_byte(uint8_t data) {
    UART__putbyte(data);
}

void send_bytes(const uint8_t* data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        UART__putbyte(data[i]);
    }
}


static void count_cb(uint8_t* cred_id, uint8_t* app_hash, void* data) {
    (void)cred_id; (void)app_hash;
    uint8_t* count_ptr = (uint8_t*)data;
    (*count_ptr)++;
}

static void send_cb(uint8_t* cred_id, uint8_t* app_hash, void* data) {
    (void)data;
    send_bytes(cred_id, CREDENTIAL_ID_SIZE);
    send_bytes(app_hash, SHA1_APP_ID_SIZE);
}


// gestion des commandes

void handle_make_credential(void) {
    // Consentement
    if (!ui_wait_for_consent()) {
        // MakeCredentialError
        send_byte(STATUS_ERR_APPROVAL);
        return;
    }
    // generer cles
    uECC_set_rng(rng_generate);
    if (!uECC_make_key(public_key, private_key, uECC_secp160r1())) {
        // MakeCredentialError
        send_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }
    // generer le credential id
    rng_generate(credential_id, CREDENTIAL_ID_SIZE);
    // sauvegarde dans l'eeprom le sha1 app_id, cred id et clé privee
    if (!storage_save(buffer_app_id, credential_id, private_key)) {
        // MakeCredentialError
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
        // GetAssertionError
        send_byte(STATUS_ERR_NOT_FOUND);
        return;
    }

    // Attendre consentement
    if (!ui_wait_for_consent()) {
        // GetAssertionError
        send_byte(STATUS_ERR_APPROVAL);
        return;
    }

    // Signer
    uECC_set_rng(rng_generate);
    if (!uECC_sign(private_key, buffer_challenge, CLIENT_DATA_HASH_SIZE, signature, uECC_secp160r1())) {
        // GetAssertionError
        send_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }

    // GetAssertionResponse:
    send_byte(STATUS_OK);
    send_bytes(credential_id, CREDENTIAL_ID_SIZE);
    send_bytes(signature, SIGNATURE_SIZE);
}


void handle_list_credentials(void) {
    uint8_t count = 0;
    storage_iterate(count_cb, &count);

    // ListCredentialsResponse:
    send_byte(STATUS_OK);
    send_byte(count);
    storage_iterate(send_cb, NULL);
}

void handle_reset(void) {
    if (!ui_wait_for_consent()) {
        // ResetError
        send_byte(STATUS_ERR_APPROVAL);
        return;
    }
    storage_reset();
    // ResetResponse:
    send_byte(STATUS_OK);
}
