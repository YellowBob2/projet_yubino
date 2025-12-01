#ifndef CONSTS_H
#define CONSTS_H

// Commandes
#define CMD_LIST_CREDENTIALS    0x00
#define CMD_MAKE_CREDENTIAL     0x01
#define CMD_GET_ASSERTION       0x02
#define CMD_RESET               0x03

// Status Codes
#define STATUS_OK                   0x00
#define STATUS_ERR_COMMAND_UNKNOWN  0x01
#define STATUS_ERR_CRYPTO_FAILED    0x02
#define STATUS_ERR_BAD_PARAMETER    0x03
#define STATUS_ERR_NOT_FOUND        0x04
#define STATUS_ERR_STORAGE_FULL     0x05
#define STATUS_ERR_APPROVAL         0x06

// Tailles (Bytes)
#define LEN_APP_ID_HASH         20
#define LEN_CREDENTIAL_ID       16
#define LEN_PUB_KEY             40
#define LEN_PRIV_KEY            21
#define LEN_SIGNATURE           40
#define LEN_CHALLENGE           20 // Taille du challenge standard dans le protocole

#endif