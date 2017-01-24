#include "serial_user.h"

SerialConfig serial_config = {
    115200,
    0,
    0,
    0
};

SerialDriver* const wifi_SD = &SD1;

uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];

void serial_set_pin(void) {
    palSetPadMode(GPIOA, GPIOA_WIFI_RX, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOA, GPIOA_WIFI_TX, PAL_MODE_ALTERNATE(7));
}
