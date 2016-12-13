#include "serial_user.h"

SerialConfig serial_cfg = {
    115200,
    0,
    USART_CR2_STOP1_BITS,
    0
};

SerialDriver* wifi_SD = &SD6;

uint8_t serial_tx_buffer[SERIAL_TX_BUFFER_SIZE];

void serial_set_pin(void) {
    palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(8));
    palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(8));
}
