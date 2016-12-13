#include "serial_user.h"

SerialConfig serial_cfg = {
    115200,
    0,
    USART_CR2_STOP1_BITS,
    0
};

SerialDriver* wifi_SD = &SD3;

uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];

void serial_set_pin(void) {
    palSetPadMode(GPIOD, 8, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOD, 9, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOD, 11, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOD, 12, PAL_MODE_ALTERNATE(7));
}
