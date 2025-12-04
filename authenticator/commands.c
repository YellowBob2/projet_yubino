#include <string.h>
#include <avr/io.h>

#include "consts.h"
#include "uart.h"
#include "ui.h"
#include "rng.h"
#include "storage.h"
#include "micro-ecc/uECC.h"
#include "commands.h"

/* --- Buffers internes au module commandes --- */
static uint8_t buffer_app_id[LEN_APP_ID_HASH];
static uint8_t buffer_challenge[LEN_CHALLENGE];
static uint8_t public_key[LEN_PUB_KEY];
static uint8_t private_key[LEN_PRIV_KEY];
static uint8_t credential_id[LEN_CREDENTIAL_ID];
static uint8_t signature[LEN_SIGNATURE];

/* --- Callbacks pour storage_iterate --- */

// Callback pour compter les identifiants
static void count_cb(uint8_t* cred_id, uint8_t* app_hash, void* data) {
    (void)cred_id;
    (void)app_hash; // Inutilisés
    uint8_t* count_ptr = (uint8_t*)data;
    (*count_ptr)++;
}

// Callback pour envoyer les identifiants via UART
static void send_cb(uint8_t* cred_id, uint8_t* app_hash, void* data) {
    (void)data; // Inutilisé
    uart_send_buffer(cred_id, LEN_CREDENTIAL_ID);
    uart_send_buffer(app_hash, LEN_APP_ID_HASH);
}

/* --- Gestionnaires de commandes (internes au module) --- */

static void handle_make_credential(void) {
    // 1. Recevoir SHA1(app_id) (20 octets)
    for (int i = 0; i < LEN_APP_ID_HASH; i++) {
        buffer_app_id[i] = uart_receive_byte();
    }

    // 2. Consentement
    if (!ui_wait_for_consent()) {
        uart_send_byte(STATUS_ERR_APPROVAL);
        return;
    }

    // 3. Crypto : Générer clés
    uECC_set_rng(rng_generate);
    if (!uECC_make_key(public_key, private_key, uECC_secp160r1())) {
        uart_send_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }

    // 4. Générer credential_id
    rng_generate(credential_id, LEN_CREDENTIAL_ID);

    // 5. Sauvegarder
    if (!storage_save(buffer_app_id, credential_id, private_key)) {
        uart_send_byte(STATUS_ERR_STORAGE_FULL);
        return;
    }

    // 6. Répondre
    uart_send_byte(STATUS_OK);
    uart_send_buffer(credential_id, LEN_CREDENTIAL_ID);
    uart_send_buffer(public_key, LEN_PUB_KEY);
}

static void handle_get_assertion(void) {
    // 1. Recevoir app_id (20) et challenge (20)
    for (int i = 0; i < LEN_APP_ID_HASH; i++) {
        buffer_app_id[i] = uart_receive_byte();
    }
    for (int i = 0; i < LEN_CHALLENGE; i++) {
        buffer_challenge[i] = uart_receive_byte();
    }

    // 2. Chercher la clé
    if (!storage_find_key(buffer_app_id, private_key, credential_id)) {
        uart_send_byte(STATUS_ERR_NOT_FOUND);
        return;
    }

    // 3. Consentement
    if (!ui_wait_for_consent()) {
        uart_send_byte(STATUS_ERR_APPROVAL);
        return;
    }

    // 4. Signer
    uECC_set_rng(rng_generate);
    if (!uECC_sign(private_key, buffer_challenge, LEN_CHALLENGE, signature, uECC_secp160r1())) {
        uart_send_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }

    // 5. Répondre
    uart_send_byte(STATUS_OK);
    uart_send_buffer(credential_id, LEN_CREDENTIAL_ID);
    uart_send_buffer(signature, LEN_SIGNATURE);
}

static void handle_list_credentials(void) {
    uint8_t count = 0;
    // Compter
    storage_iterate(count_cb, &count);

    // Répondre Header
    uart_send_byte(STATUS_OK);
    uart_send_byte(count);

    // Envoyer liste
    storage_iterate(send_cb, NULL);
}

static void handle_reset(void) {
    if (!ui_wait_for_consent()) {
        uart_send_byte(STATUS_ERR_APPROVAL);
        return;
    }
    storage_reset();
    uart_send_byte(STATUS_OK);
}

/* --- Fonction publique --- */

void command_handle(uint8_t cmd) {
    switch (cmd) {
        case CMD_MAKE_CREDENTIAL:
            handle_make_credential();
            break;
        case CMD_GET_ASSERTION:
            handle_get_assertion();
            break;
        case CMD_LIST_CREDENTIALS:
            handle_list_credentials();
            break;
        case CMD_RESET:
            handle_reset();
            break;
        default:
            uart_send_byte(STATUS_ERR_COMMAND_UNKNOWN);
            break;
    }
}


