#include <avr/sleep.h>

#include "uart.h"
#include "consts.h"
#include "ui.h"
#include "rng.h"
#include "globals.h"
#include "commands.h"


int main(void) {
    UART__init();
    ui_init();
    //rng_set_method(RNG_METHOD_ADC); // tester plusieurs
    rng_init(); // defaut init methode combinee
    //storage_init();
    set_sleep_mode(SLEEP_MODE_IDLE);

    while (1) {
        uint8_t cmd;

        if (UART__getbyte(&cmd) == 0) {
            switch (cmd) {

                case COMMAND_MAKE_CREDENTIAL: {
                    if (read_bytes_with_timeout(buffer_app_id, SHA1_APP_ID_SIZE, 1000) == 0) {
                        // MakeCredentialError
                        send_byte(STATUS_ERR_BAD_PARAMETER);
                    } else {
                        handle_make_credential();
                    }
                    break;
                }

                case COMMAND_GET_ASSERTION: {
                    if (read_bytes_with_timeout(buffer_app_id, SHA1_APP_ID_SIZE, 1000) == 0 ||
                        read_bytes_with_timeout(buffer_challenge, CLIENT_DATA_HASH_SIZE, 1000) == 0) {
                        // GetAssertionError
                        send_byte(STATUS_ERR_BAD_PARAMETER);
                    } else {
                        handle_get_assertion();
                    }
                    break;
                }

                case COMMAND_LIST_CREDENTIALS: {
                    handle_list_credentials();
                    break;
                }

                case COMMAND_RESET: {
                    handle_reset();
                    break;
                }

                default: {
                    send_byte(STATUS_ERR_COMMAND_UNKNOWN);
                    break;
                }
            }

        } else {
            UART__sleep();
        }
    }
    return 0;
}
