#include "serial_user.h"

SerialConfig serial_cfg = {
    115200,
    0,
    0,
    0
};

SerialDriver* const wifi_SD = &SD4;

uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];

void serial_set_pin(void) {
    palSetPadMode(GPIOC, 10, PAL_MODE_ALTERNATE(8));
    palSetPadMode(GPIOC, 11, PAL_MODE_ALTERNATE(8));
}
