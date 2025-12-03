#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

void handle_make_credential(void);
void handle_get_assertion(void);
void handle_list_credentials(void);
void handle_reset(void);
void send_byte(uint8_t data);
void send_bytes(const uint8_t* data, uint16_t len);
uint8_t read_bytes_with_timeout(uint8_t* buffer, uint8_t length, uint16_t timeout_ms);

#endif
