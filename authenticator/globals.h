#ifndef GLOBALS_H
#define GLOBALS_H

// Buffers globaux
uint8_t buffer_app_id[SHA1_APP_ID_SIZE];
uint8_t buffer_challenge[CLIENT_DATA_HASH_SIZE];
uint8_t public_key[PUBLIC_KEY_SIZE];
uint8_t private_key[PRIVATE_KEY_SIZE];
uint8_t credential_id[CREDENTIAL_ID_SIZE];
uint8_t signature[SIGNATURE_SIZE];

#endif
