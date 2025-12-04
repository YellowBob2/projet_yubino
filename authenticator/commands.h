#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

/**
 * Traite une commande reçue via UART.
 *
 * @param cmd octet de commande (voir les constantes CMD_* dans consts.h)
 */
void command_handle(uint8_t cmd);

#endif


