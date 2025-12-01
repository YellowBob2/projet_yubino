#ifndef UI_H
#define UI_H
#include <stdint.h>

void ui_init(void);
// Retourne 1 si consentement donné, 0 si timeout (10s)
uint8_t ui_wait_for_consent(void);

#endif