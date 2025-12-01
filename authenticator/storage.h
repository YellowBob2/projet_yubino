/* storage.h */
#ifndef STORAGE_H
#define STORAGE_H
#include <stdint.h>
#include "consts.h"

typedef struct {
    uint8_t used; // 0xFF si vide, 0x01 si utilisé
    uint8_t app_id_hash[LEN_APP_ID_HASH];
    uint8_t credential_id[LEN_CREDENTIAL_ID];
    uint8_t private_key[LEN_PRIV_KEY];
} CredentialEntry;

void storage_init(void);

void storage_reset(void);

uint8_t storage_save(const uint8_t* app_id_hash, const uint8_t* cred_id, const uint8_t* priv_key);

uint8_t storage_find_key(const uint8_t* app_id_hash, uint8_t* priv_key_out, uint8_t* cred_id_out);

// Le callback accepte maintenant un pointeur void* (le contexte)
void storage_iterate(void (*callback)(uint8_t* cred_id, uint8_t* app_hash, void* data), void* data);

#endif