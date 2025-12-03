#ifndef UI_H
#define UI_H

#include <stdint.h>

void ui_init(void);
uint8_t ui_wait_for_consent(void);
void ui_sleep_tick(uint16_t last_ms);
uint16_t ui_get_ms(void);


#endif
