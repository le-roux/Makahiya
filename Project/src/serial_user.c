#include "serial_user.h"

SerialConfig serial_cfg = {
    256000,
    0,
    0,
    0
};

SerialDriver* const wifi_SD = &SD1;

uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];

void serial_set_pin(void) {
    palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(8));
    palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(8));

    // Interrupt for websockets
    palSetPadMode(GPIOA, 11, PAL_MODE_INPUT_PULLDOWN);
}
