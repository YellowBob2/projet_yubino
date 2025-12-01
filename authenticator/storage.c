#include "storage.h"
#include <avr/eeprom.h>
#include <string.h>

// Calculer combien d'entrées tiennent dans 1KB (ATmega328P)
// Taille entry ~= 1 + 20 + 16 + 21 = 58 bytes. 1024 / 58 ~= 17 entrées.
#define MAX_ENTRIES 15
#define ENTRY_SIZE sizeof(CredentialEntry)

EEMEM CredentialEntry eeprom_entries[MAX_ENTRIES];

void storage_reset(void) {
    // Crée un buffer de zéros de la taille d'une entrée
    uint8_t zero_buffer[ENTRY_SIZE] = {0}; 

    // Remplacer TOUS les octets de toutes les entrées par des zéros.
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        // Adresse de début de l'entrée dans l'EEPROM
        CredentialEntry *entry_addr = &eeprom_entries[i];
        
        // Écrire le buffer de zéros sur toute la taille de l'entrée
        eeprom_write_block(zero_buffer, entry_addr, ENTRY_SIZE);
    }
}

uint8_t storage_save(const uint8_t* app_id_hash, const uint8_t* cred_id, const uint8_t* priv_key) {
    int8_t free_slot = -1;
    int8_t existing_slot = -1;

    // Vérifier si l'app_id existe déjà (pour remplacement) ou trouver slot vide
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        uint8_t used = eeprom_read_byte(&eeprom_entries[i].used);
        if (used == 0x01) {
            uint8_t stored_hash[LEN_APP_ID_HASH];
            eeprom_read_block(stored_hash, eeprom_entries[i].app_id_hash, LEN_APP_ID_HASH);
            if (memcmp(stored_hash, app_id_hash, LEN_APP_ID_HASH) == 0) {
                existing_slot = i;
                break;
            }
        } else if (free_slot == -1) {
            free_slot = i;
        }
    }

    int8_t target = (existing_slot != -1) ? existing_slot : free_slot;

    if (target == -1) return 0; // Storage Full

    // Écriture
    uint8_t flag = 0x01;
    eeprom_write_byte(&eeprom_entries[target].used, flag);
    eeprom_write_block(app_id_hash, eeprom_entries[target].app_id_hash, LEN_APP_ID_HASH);
    eeprom_write_block(cred_id, eeprom_entries[target].credential_id, LEN_CREDENTIAL_ID);
    eeprom_write_block(priv_key, eeprom_entries[target].private_key, LEN_PRIV_KEY);

    return 1;
}

uint8_t storage_find_key(const uint8_t* app_id_hash, uint8_t* priv_key_out, uint8_t* cred_id_out) {
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (eeprom_read_byte(&eeprom_entries[i].used) == 0x01) {
            uint8_t stored_hash[LEN_APP_ID_HASH];
            eeprom_read_block(stored_hash, eeprom_entries[i].app_id_hash, LEN_APP_ID_HASH);
            if (memcmp(stored_hash, app_id_hash, LEN_APP_ID_HASH) == 0) {
                eeprom_read_block(priv_key_out, eeprom_entries[i].private_key, LEN_PRIV_KEY);
                if(cred_id_out) eeprom_read_block(cred_id_out, eeprom_entries[i].credential_id, LEN_CREDENTIAL_ID);
                return 1;
            }
        }
    }
    return 0;
}

void storage_iterate(void (*callback)(uint8_t* cred_id, uint8_t* app_hash, void* data), void* data) {
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (eeprom_read_byte(&eeprom_entries[i].used) == 0x01) {
             uint8_t c_id[LEN_CREDENTIAL_ID];
             uint8_t a_hash[LEN_APP_ID_HASH];
             eeprom_read_block(c_id, eeprom_entries[i].credential_id, LEN_CREDENTIAL_ID);
             eeprom_read_block(a_hash, eeprom_entries[i].app_id_hash, LEN_APP_ID_HASH);
             // Le contexte est passé
             callback(c_id, a_hash, data);
        }
    }
}